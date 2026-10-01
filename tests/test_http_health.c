#include <check.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define HEALTH_HOST "localhost"
#define HEALTH_PORT "8008"
#define RESPONSE_BUFFER_SIZE 8192

START_TEST(test_http_health) {
    // Create a socket and connect to the health endpoint
    struct addrinfo hints = {0};
    struct addrinfo *addresses = NULL;
    int socket_fd = -1;
    char response[RESPONSE_BUFFER_SIZE];
    size_t response_length = 0;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int result = getaddrinfo(HEALTH_HOST, HEALTH_PORT, &hints, &addresses);
    ck_assert_msg(result == 0, "Could not resolve %s: %s", HEALTH_HOST,
                  gai_strerror(result));

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
    ck_assert_msg(socket_fd != -1, "Could not connect to %s:%s", HEALTH_HOST,
                  HEALTH_PORT);

    const char request[] =
            "GET /health HTTP/1.1\r\n"
            "Host: localhost:8008\r\n"
            "Connection: close\r\n"
            "\r\n";

    const ssize_t sent = send(socket_fd, request, sizeof(request) - 1, 0);
    ck_assert_msg(sent == (ssize_t)(sizeof(request) - 1), "Could not send health request");

    while (response_length < sizeof(response) - 1) {
        const ssize_t received = recv(socket_fd, response + response_length,
        sizeof(response) - 1 - response_length, 0);

        if (received <= 0) break;
        
        response_length += (size_t) received;
    }
    close(socket_fd);
    response[response_length] = '\0';

    char const *body = strstr(response, "\r\n\r\n");
    ck_assert_msg(body != NULL, "HTTP response did not contain a body");
    body += 4;
    ck_assert_str_eq(body, "OK");
}

END_TEST

int main(void) {
    Suite *suite = suite_create("HTTP Health Test Suite");
    TCase *test_case = tcase_create("HTTP Health Test Case");

    tcase_add_test(test_case, test_http_health);
    suite_add_tcase(suite, test_case);

    SRunner *runner = srunner_create(suite);
    // Disable forking to ensure the test runs in the same process
    srunner_set_fork_status(runner, CK_NOFORK);
    srunner_run_all(runner, CK_VERBOSE);

    const int number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
