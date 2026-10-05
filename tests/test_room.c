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

#define TEST_USERNAME "dirk"
#define TEST_PASSWORD "dirk"
#define TEST_ROOM_NAME "Test room"

// Connect to Synapse and return the socket
static int connect_to_synapse(void) {
    struct addrinfo hints = {0};
    struct addrinfo *addresses = NULL;
    int socket_fd = -1;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    const int result = getaddrinfo(SYNAPSE_HOST, SYNAPSE_PORT, &hints, &addresses);
    ck_assert_msg(result == 0, "Could not resolve %s: %s", SYNAPSE_HOST, gai_strerror(result));

    for (const struct addrinfo *address = addresses; address != NULL; address = address->ai_next) {
        socket_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        // If socket creation fails, continue to the next address
        if (socket_fd == -1) continue;
        // If connection fails, close the socket and continue to the next address
        if (connect(socket_fd, address->ai_addr, address->ai_addrlen) == 0) break;

        close(socket_fd);
        socket_fd = -1;
    }

    freeaddrinfo(addresses);
    ck_assert_msg(socket_fd != -1, "Could not connect to %s:%s", SYNAPSE_HOST, SYNAPSE_PORT);

    return socket_fd;
}

// Send the JSON POST request and store the HTTP response in `response`
static void post_json(const char *path, const char *token, const char *json_body, char *response,
                      const size_t response_size) {
    char request[REQUEST_BUFFER_SIZE];
    char auth_header[TOKEN_BUFFER_SIZE + 32] = "";
    size_t response_length = 0;

    if (token != NULL) {
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s\r\n", token);
    }

    const int request_length = snprintf(request, sizeof(request),
                                        "POST %s HTTP/1.1\r\n"
                                        "Host: %s:%s\r\n"
                                        "%s"
                                        "Content-Type: application/json\r\n"
                                        "Content-Length: %zu\r\n"
                                        "Connection: close\r\n"
                                        "\r\n"
                                        "%s",
                                        path, SYNAPSE_HOST, SYNAPSE_PORT, auth_header, strlen(json_body), json_body);
    ck_assert_msg(request_length > 0 && (size_t) request_length < sizeof(request),
                  "HTTP request did not fit in the request buffer");

    const int socket_fd = connect_to_synapse();

    size_t total_sent = 0;
    while (total_sent < (size_t) request_length) {
        const ssize_t sent = send(socket_fd, request + total_sent, (size_t) request_length - total_sent, 0);
        ck_assert_msg(sent > 0, "Could not send request to %s", path);
        total_sent += (size_t) sent;
    }

    while (response_length < response_size - 1) {
        const ssize_t received = recv(socket_fd, response + response_length, response_size - response_length - 1, 0);
        if (received <= 0) break;
        response_length += (size_t) received;
    }
    close(socket_fd);
    response[response_length] = '\0';
}

// Parse the HTTP status code, return -1 if malformed
static int http_status_code(const char *response) {
    int status = -1;
    if (sscanf(response, "HTTP/%*d.%*d %d", &status) != 1) return -1;
    return status;
}

// Return pointer to the start of the HTTP body, or NULL if there is none
static const char *http_body(const char *response) {
    const char *body = strstr(response, "\r\n\r\n");
    return body == NULL ? NULL : body + 4;
}

// Parse the JSON and take the value of the first string: "access_token":"abc".
// Return 1 on success and 0 if the key is missing or not a string.
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

// Log in as the test user and return the access token via `token`
static void login(char *token, const size_t token_size) {
    char response[RESPONSE_BUFFER_SIZE];

    const char login_body[] =
            "{"
            "\"identifier\":{\"type\":\"m.id.user\",\"user\":\"" TEST_USERNAME "\"},"
            "\"password\":\"" TEST_PASSWORD "\","
            "\"type\":\"m.login.password\""
            "}";

    post_json("/_matrix/client/v3/login", NULL, login_body, response, sizeof(response));

    ck_assert_msg(http_status_code(response) == 200, "Login did not return 200 OK. Response:\n%s", response);

    const char *body = http_body(response);
    ck_assert_msg(body != NULL, "Login response did not contain a body");
    ck_assert_msg(json_get_string(body, "access_token", token, token_size),
                  "Login response did not contain an access_token. Body:\n%s", body);
}

START_TEST(test_room_create) {
    char access_token[TOKEN_BUFFER_SIZE];
    char response[RESPONSE_BUFFER_SIZE];
    char room_id[ROOM_ID_BUFFER_SIZE];

    login(access_token, sizeof(access_token));

    const char create_room_body[] = "{\"name\":\"" TEST_ROOM_NAME "\"}";
    post_json("/_matrix/client/v3/createRoom", access_token, create_room_body, response, sizeof(response));

    // Assert 200 OK is returned
    const int status = http_status_code(response);
    ck_assert_msg(status == 200, "Room creation returned status %d instead of 200. Response:\n%s", status, response);

    // Assert a non-empty room_id is given
    const char *body = http_body(response);
    ck_assert_msg(body != NULL, "Room creation response did not contain a body");
    ck_assert_msg(json_get_string(body, "room_id", room_id, sizeof(room_id)),
                  "Room creation response did not contain a room_id. Body:\n%s", body);
    ck_assert_msg(room_id[0] == '!', "room_id '%s' does not start with '!'", room_id);
}
END_TEST

int main(void) {
    Suite *suite = suite_create("Room Test Suite");
    TCase *test_cases = tcase_create("Room Creation Test Case");

    tcase_add_test(test_cases, test_room_create);
    suite_add_tcase(suite, test_cases);

    SRunner *runner = srunner_create(suite);
    // Disable forking to ensure the test runs in the same process
    srunner_set_fork_status(runner, CK_NOFORK);
    srunner_run_all(runner, CK_VERBOSE);

    const int number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
