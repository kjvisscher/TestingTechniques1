#include <check.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SYNAPSE_HOST "localhost"
#define SYNAPSE_PORT "8008"
#define RESPONSE_BUFFER_SIZE 8192
#define REQUEST_BUFFER_SIZE 2048
#define TOKEN_BUFFER_SIZE 512
#define ROOM_ID_BUFFER_SIZE 256
#define PATH_BUFFER_SIZE 512

#define TEST_USERNAME "dirk"
#define TEST_PASSWORD "dirk"
#define TEST_ROOM_NAME "Test room"

#define VALID_MESSAGE "{\"body\":\"Test message\",\"msgtype\":\"m.text\",\"sender\":\"@" TEST_USERNAME ":localhost\"}"

// Shared access token
static char access_token[TOKEN_BUFFER_SIZE] = "";
static char test_room_id[ROOM_ID_BUFFER_SIZE] = "";

#pragma region HTTP Helpers

// Connect to Synapse and return the socket
static int connect_to_synapse(void) {
    struct addrinfo hints = {0};
    struct addrinfo *addresses = NULL;
    int socket_fd = -1;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(SYNAPSE_HOST, SYNAPSE_PORT, &hints, &addresses) != 0) return -1;

    for (const struct addrinfo *address = addresses; address != NULL; address = address->ai_next) {
        socket_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        // If socket creation fails, continue to next address
        if (socket_fd == -1) continue;
        // If connection fails, close socket and continue to next address
        if (connect(socket_fd, address->ai_addr, address->ai_addrlen) == 0) break;

        close(socket_fd);
        socket_fd = -1;
    }

    freeaddrinfo(addresses);
    return socket_fd;
}

// Parse the HTTP status code
static int http_status_code(const char *response) {
    int status = -1;
    if (sscanf(response, "HTTP/%*d.%*d %d", &status) != 1) return -1;
    return status;
}

// Get pointer to the start of the HTTP body
static const char *http_body(const char *response) {
    const char *body = strstr(response, "\r\n\r\n");
    return body == NULL ? NULL : body + 4;
}

// Parse the JSON and take the value of the first string: "access_token":"abc"
static int json_get_string(const char *json, const char *key, char *out, const size_t out_size) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    const char *cursor = strstr(json, pattern);
    if (cursor == NULL) return 0;
    cursor += strlen(pattern);

    while (isspace((unsigned char) *cursor)) cursor++;
    if (*cursor != ':') return 0;
    cursor++;

    while (isspace((unsigned char) *cursor)) cursor++;
    if (*cursor != '"') return 0;
    cursor++;

    const char *end = strchr(cursor, '"');
    if (end == NULL) return 0;

    const size_t length = (size_t) (end - cursor);
    if (length == 0 || length >= out_size) return 0;

    memcpy(out, cursor, length);
    out[length] = '\0';
    return 1;
}

// Send the HTTP request and store the response
static int try_send_request(const char *method, const char *path, const char *token, const char *json_body,
                            char *response, const size_t response_size) {
    char request[REQUEST_BUFFER_SIZE];
    char auth_header[TOKEN_BUFFER_SIZE + 32] = "";
    char content_headers[96] = "";
    size_t response_length = 0;

    response[0] = '\0';

    if (token != NULL) {
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s\r\n", token);
    }
    if (json_body != NULL) {
        snprintf(content_headers, sizeof(content_headers), "Content-Type: application/json\r\nContent-Length: %zu\r\n",
                 strlen(json_body));
    }

    const int request_length = snprintf(request, sizeof(request),
                                        "%s %s HTTP/1.1\r\n"
                                        "Host: %s:%s\r\n"
                                        "%s"
                                        "%s"
                                        "Connection: close\r\n"
                                        "\r\n"
                                        "%s",
                                        method, path, SYNAPSE_HOST, SYNAPSE_PORT, auth_header, content_headers,
                                        json_body != NULL ? json_body : "");
    if (request_length <= 0 || (size_t) request_length >= sizeof(request)) return -1;

    const int socket_fd = connect_to_synapse();
    if (socket_fd == -1) return -1;

    size_t total_sent = 0;
    while (total_sent < (size_t) request_length) {
        const ssize_t sent = send(socket_fd, request + total_sent, (size_t) request_length - total_sent, 0);
        if (sent <= 0) {
            close(socket_fd);
            return -1;
        }
        total_sent += (size_t) sent;
    }
    while (response_length < response_size - 1) {
        const ssize_t received = recv(socket_fd, response + response_length, response_size - response_length - 1, 0);
        if (received <= 0) break;
        response_length += (size_t) received;
    }
    close(socket_fd);
    response[response_length] = '\0';
    return 0;
}

// Send HTTP request in a test and fail the test if it cannot be sent
static void send_request(const char *method, const char *path, const char *token, const char *json_body, char *response,
                         const size_t response_size) {
    ck_assert_msg(try_send_request(method, path, token, json_body, response, response_size) == 0,
                  "Could not send %s request to %s:%s%s", method, SYNAPSE_HOST, SYNAPSE_PORT, path);
}
#pragma endregion

#pragma region Assertion Helpers
static void assert_status(const char *response, const int expected_status) {
    const int status = http_status_code(response);
    ck_assert_msg(status == expected_status, "Expected HTTP status %d but got %d",
                  expected_status, status);
}

static const char *assert_body(const char *response) {
    const char *body = http_body(response);
    ck_assert_msg(body != NULL, "HTTP response did not contain a body");
    return body;
}

// Assert a 200 OK response that contains event_id
static void assert_event_sent(const char *response) {
    char event_id[ROOM_ID_BUFFER_SIZE];

    assert_status(response, 200);
    const char *body = assert_body(response);
    ck_assert_msg(json_get_string(body, "event_id", event_id, sizeof(event_id)),
                  "Response did not contain an event_id. Body:\n%s", body);
    ck_assert_msg(event_id[0] == '$', "event_id '%s' does not start with '$'", event_id);
}

// Assert error response with the given status and Matrix errcode
static void assert_error(const char *response, const int expected_status, const char *expected_errcode) {
    char errcode[64];

    assert_status(response, expected_status);
    const char *body = assert_body(response);
    ck_assert_msg(json_get_string(body, "errcode", errcode, sizeof(errcode)),
                  "Response did not contain an errcode. Body:\n%s", body);
    ck_assert_str_eq(errcode, expected_errcode);
}
#pragma endregion

#pragma region Matrix Helpers
// Log in as user and save the authentication token
static int login(char *token, const size_t token_size) {
    char response[RESPONSE_BUFFER_SIZE];

    token[0] = '\0';

    const char login_body[] = "{\"identifier\":{\"type\":\"m.id.user\",\"user\":\"" TEST_USERNAME "\"},"
            "\"password\":\"" TEST_PASSWORD "\","
            "\"type\":\"m.login.password\"}";

    if (try_send_request("POST", "/_matrix/client/v3/login", NULL, login_body, response, sizeof(response)) != 0) {
        fprintf(stderr, "Login failed: could not reach Synapse at %s:%s\n", SYNAPSE_HOST, SYNAPSE_PORT);
        return 0;
    }

    const int status = http_status_code(response);
    if (status != 200) {
        fprintf(stderr, "Login failed: expected HTTP status 200 but got %d\n", status);
        return 0;
    }

    const char *body = http_body(response);
    if (body == NULL || !json_get_string(body, "access_token", token, token_size)) {
        fprintf(stderr, "Login failed: response did not contain an access_token\n");
        token[0] = '\0';
        return 0;
    }

    return 1;
}

// Remove rate limit
static int remove_rate_limit(const char *token) {
    char response[RESPONSE_BUFFER_SIZE];
    const char rate_limit_body[] = "{\"messages_per_second\": 0, \"burst_count\": 0}";

    if (try_send_request("POST", "/_synapse/admin/v1/users/@" TEST_USERNAME ":localhost/override_ratelimit", token, rate_limit_body, response, sizeof(response)) != 0) {
        fprintf(stderr, "Remove rate limit failed: could not reach Synapse at %s:%s\n", SYNAPSE_HOST, SYNAPSE_PORT);
        return 0;
    }

    const int status = http_status_code(response);
    if (status != 200) {
        fprintf(stderr, "Remove rate limit failed: expected HTTP status 200 but got %d\n", status);
        return 0;
    }

    return 1;
}

// Create a new room and return its ID
static void create_room(const char *token, char *room_id, const size_t room_id_size) {
    char response[RESPONSE_BUFFER_SIZE];

    const char create_room_body[] = "{\"name\":\"" TEST_ROOM_NAME "\"}";
    send_request("POST", "/_matrix/client/v3/createRoom", token, create_room_body, response, sizeof(response));
    assert_status(response, 200);

    const char *body = assert_body(response);
    ck_assert_msg(json_get_string(body, "room_id", room_id, room_id_size),
                  "Room creation response did not contain a room_id. Body:\n%s", body);
}

// Build the path of the endpoint for the given room
static void message_path(char *path, const size_t path_size, const char *room_id) {
    snprintf(path, path_size, "/_matrix/client/v3/rooms/%s/state/m.room.message/", room_id);
}

// Send a message to a room using the regular send message request
static void send_message(const char *room_id, const char *token, const char *json_body, char *response,
                         const size_t response_size) {
    char path[PATH_BUFFER_SIZE];

    message_path(path, sizeof(path), room_id);
    send_request("PUT", path, token, json_body, response, response_size);
}


static void setup(void) {
    create_room(access_token, test_room_id, sizeof(test_room_id));
}
#pragma endregion

#pragma region Testcases
// Valid message sent request is accepted
START_TEST(test_message_send) {
    char response[RESPONSE_BUFFER_SIZE];

    send_message(test_room_id, access_token, VALID_MESSAGE, response, sizeof(response));

    assert_event_sent(response);
}

END_TEST

// Bad access token
START_TEST(test_message_send_bad_token) {
    char response[RESPONSE_BUFFER_SIZE];

    send_message(test_room_id, "not_a_valid_token", VALID_MESSAGE, response, sizeof(response));

    assert_error(response, 401, "M_UNKNOWN_TOKEN");
}

END_TEST

// Endpoint does not exist
START_TEST(test_message_send_bad_endpoint) {
    char response[RESPONSE_BUFFER_SIZE];
    char path[PATH_BUFFER_SIZE];

    snprintf(path, sizeof(path), "/_matrix/client/v3/rooms/%s/state_typo/m.room.message/", test_room_id);
    send_request("PUT", path, access_token, VALID_MESSAGE, response, sizeof(response));

    assert_error(response, 404, "M_UNRECOGNIZED");
}

END_TEST

// Endpoint with wrong HTTP method
START_TEST(test_message_send_wrong_method) {
    char response[RESPONSE_BUFFER_SIZE];
    char path[PATH_BUFFER_SIZE];

    message_path(path, sizeof(path), test_room_id);
    send_request("POST", path, access_token, VALID_MESSAGE, response, sizeof(response));

    const int status = http_status_code(response);
    ck_assert_msg(status == 405, "Expected HTTP status 404 or 405 but got %d", status);
    assert_error(response, status, "M_UNRECOGNIZED");
}

END_TEST

// Valid message with empty body
START_TEST(test_message_send_empty_message) {
    char response[RESPONSE_BUFFER_SIZE];

    send_message(test_room_id, access_token, "{\"body\":\"\",\"msgtype\":\"m.text\"}", response, sizeof(response));

    assert_event_sent(response);
}

END_TEST

// Message body has wrong JSON type, rejected because 'body' not a string type
START_TEST(test_message_send_wrong_type) {
    char response[RESPONSE_BUFFER_SIZE];

    send_message(test_room_id, access_token, "{\"body\":12345,\"msgtype\":\"m.text\"}", response, sizeof(response));

    assert_error(response, 400, "M_UNKNOWN");
}

END_TEST

// Sender in content doesn't match token, accepted as sender field is ignored
START_TEST(test_message_send_invalid_sender) {
    char response[RESPONSE_BUFFER_SIZE];

    send_message(test_room_id, access_token,
                 "{\"body\":\"Test message\",\"msgtype\":\"m.text\",\"sender\":\"@nobody:localhost\"}",
                 response, sizeof(response));

    assert_event_sent(response);
}

END_TEST

// Malformed JSON is rejected
START_TEST(test_message_send_invalid_json) {
    char response[RESPONSE_BUFFER_SIZE];

    send_message(test_room_id, access_token, "{\"body\":\"Test message\",", response, sizeof(response));

    assert_error(response, 400, "M_NOT_JSON");
}

END_TEST

// Unknown field msgtype, accepted because Matrix allows custom message types
START_TEST(test_message_send_unknown_msgtype) {
    char response[RESPONSE_BUFFER_SIZE];

    send_message(test_room_id, access_token, "{\"body\":\"Test message\",\"msgtype\":\"m.does_not_exist\"}",
                 response, sizeof(response));

    assert_event_sent(response);
}

END_TEST

// Room does not exist
START_TEST(test_message_send_unknown_room) {
    char response[RESPONSE_BUFFER_SIZE];

    send_message("!doesnotexist:localhost", access_token, VALID_MESSAGE, response, sizeof(response));

    assert_error(response, 403, "M_FORBIDDEN");
}

END_TEST
#pragma endregion

int main(void) {
    login(access_token, sizeof(access_token));
    remove_rate_limit(access_token);

    Suite *suite = suite_create("Message Test Suite");
    TCase *test_cases = tcase_create("Message Send Test Case");

    tcase_add_checked_fixture(test_cases, setup, NULL);
    tcase_add_test(test_cases, test_message_send);
    tcase_add_test(test_cases, test_message_send_bad_token);
    tcase_add_test(test_cases, test_message_send_bad_endpoint);
    tcase_add_test(test_cases, test_message_send_wrong_method);
    tcase_add_test(test_cases, test_message_send_empty_message);
    tcase_add_test(test_cases, test_message_send_wrong_type);
    tcase_add_test(test_cases, test_message_send_invalid_sender);
    tcase_add_test(test_cases, test_message_send_invalid_json);
    tcase_add_test(test_cases, test_message_send_unknown_msgtype);
    tcase_add_test(test_cases, test_message_send_unknown_room);
    suite_add_tcase(suite, test_cases);

    SRunner *runner = srunner_create(suite);
    // Disable forking to ensure the tests run in the same process
    srunner_set_fork_status(runner, CK_NOFORK);
    srunner_run_all(runner, CK_VERBOSE);

    const int number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
