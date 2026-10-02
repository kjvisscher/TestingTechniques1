# Test Preparation
## Exercise 1: Description of SUT
For our system-under-test we have picked [Synapse](https://github.com/element-hq/synapse) a (home)server developed by [Element](https://element.io/). Synapse is an open source instance of [Matrix](https://matrix.org/) written in Python.

Using Synapse, the user can locally run the server and connect to it using a web client on [app.element.io](https://app.element.io/). These enable chat-based communication in accordance with the Matrix communication protocol.

As a chat server, Synapse allows a user to communicate following the Matrix protocol. 
- Functionality includes user registration and login, creating/joining/leaving rooms, sending and receiving messages and viewing message history. 
- The interface is the Matrix Client-Server HTTP API exposed by Synapse at `/_matrix/client/*` on `localhost:8008`, accessed either through `app.element.io` or directly via HTTP requests.
- The inputs include user credentials (registration/login), room actions (create, join, invite), and messages, entered through the `app.element.io` GUI or sent as plaintext JSON in HTTP requests.
- The outputs consist of rooms and messages rendered in the `app.element.io` UI, or viewed as raw JSON responses and HTTP status codes when interacting with the API directly.
- Synapse runs in a Docker container built from source on the tester's machine. Element runs as a web client in a browser, connecting to the local Synapse instance over HTTP.
- The server is started with `docker run` (built beforehand with `docker build`) and stopped with `docker stop`/`docker rm`; the client is simply opened/closed in a browser tab, with the homeserver URL matching that of port exposed by Docker (see [[TestingTechniques1/README]]).
==TODO: add picture of external view.==

For black-box testing the SUT's internal structure, we mainly focus on the components which have an observable form of output:
- HTTP Listener on port `8008` passing request based on content and path.
- Client-server and API handler process requests in accordance with the Matrix communication protocol.
==TODO: add picture of internal view.==

The latest version of the Synapse repository ([1.161.0](https://github.com/element-hq/synapse/releases/tag/v1.161.0)) is running as a docker container on the [latest image version](https://hub.docker.com/r/matrixdotorg/synapse). The repository and image are both run locally on either Linux or Windows operating systems. A basic setup procedure is required before running Synapse for the first time, as detailed in [[TestingTechniques1/README]].

References to relevant documentation:
- Matrix documentation: https://matrix.org/docs
- Latest Matrix API: https://spec.matrix.org/latest
- Used Synapse image: https://hub.docker.com/r/matrixdotorg/synapse
- Public Synapse repository: https://github.com/element-hq/synapse
## Exercise 2: What part of Synapse to test
We're going to test chatting between two users in a chatroom. By recreating chatrooms, we can create identical test conditions without needing to reset the container between runs. We intend to test the part dealing with chatting in a room primairly from the perspective of the user interaction that happens on the server. This means we need to test the components dealing with client-server API and room events. 
We intend to test the following features/functionality: 
1. Sending a message
2. Deleting a message
3. Replying to a message
4. Removing a reply 

The interface utilized during testing will be the client-server API. Relevant documentation can be found in the [Matrix specification](https://spec.matrix.org/v1.19/client-server-api/). Refer to the following chapters:

| Feature               | Chapter                                                                                 |
| --------------------- | --------------------------------------------------------------------------------------- |
| Errors                | [Chapter 1.1](https://spec.matrix.org/v1.19/client-server-api/#standard-error-response) |
| Room events           | [Chapter 7](https://spec.matrix.org/v1.19/client-server-api/#events)                    |
| Deleting a message    | [Chapter 7.9](https://spec.matrix.org/v1.19/client-server-api/#redactions)              |
| Removing a reply      | [Chapter 7.9](https://spec.matrix.org/v1.19/client-server-api/#redactions)              |
| Sending a message     | [Chapter 10.2](https://spec.matrix.org/v1.19/client-server-api/#instant-messaging)      |
| Replying to a message | [Chapter 10.3](https://spec.matrix.org/v1.19/client-server-api/#rich-replies)           |
> *Note that sending a message and replying to it are both events and thus both are redacted the same way.*
## Exercise 3: Test architecture for the testing
Illustrated below is a hybrid high level component view and class diagram of our testing application in relation to the SUT. We have decided to picture Synapse as a black box, as all test cases are communicated over the same protocol and are handled in the same interface.

![Diagram](component_diagram.drawio.png)

The test will be run by an automatic testing application, this application will have multiple test suits which will all test a specific group of tests. Each test suite will contain multiple test cases. Each test case will consist of a unit test like assertion. Each test case will consist of a HTTP-request which will be sent to the SUT, after which the response will be compared to the "expected behaviour" response and making the test either pass or fail.

The output of the SUT will be in compliance with the matrix communication protocol over HTTP-requests. The output will consist of text-based messagers or files. The primary focus of the tests will be text-based messagers because the comparison is simpler to assert. These responses are analyzed by the testing application.

We make the following assumptions about Synapse:
1. By recreating rooms for each test, the behaviour of messages is consistent throughout.
2. The behaviour of Synapse is identical wether it is run locally or when running on a public server. That is, the test results for achieved from running Synapse as a standalone Docker image are assumed to match those achieved from testing a "real" installation connected to multiple other servers over the internet.

The primary testing tool used for this project is a unit testing framework for C called *Check* by Arien Malec (see [repository](https://github.com/libcheck/check)). 
## Exercise 4: What typical test cases look like
The initial state for our tests will be a chat room with (at least) 2 users, with possibly extra requirements depending on the test. Test input and output are plaintext JSON objects transmitted over HTTP. Test input requires some sort of event (such as sending a message, replying to a message, etc.) as input alongside additional required parameters as specified by the Matrix API. Test output will include a HTTP status code depending on the success of the request, alongside a body which includes plaintext information or possible errors. We can observe whether a test has succeeded by comparing the received response with an expected response and asserting that:
1. The HTTP status codes match
2. Relevant fields in the JSON body (i.e. message content) match.


- b) how will you specify your test cases, i.e., the test notation or language to document your test cases; give a template for a test case.
==TODO Exercise 4b==
## Exercise 5: Discription implemented test architecture
- a) concrete implementation of input and output interfaces; 
- b) implementation of stubs, drivers, used test tools; 
- c) any additional tools or implementation details that are necessary; 
- d) are all test interfaces in the architecture accessible?
# Test Development
## Exercise 6: Domains, inputs and interfaces
What are the domains of test inputs and outputs, what are valid and invalid inputs, and over which interfaces are they communicated?

## Exercise 7: Black-box functionality test cases
Develop (at least) 12 black-box functionality test cases to test your sut and write them in your test notation. Motivate your choice for these test cases, and make clear which test generation technique you used for each test (EP, BVA, state-based, use-case, . . . ).
# Test Execution
## Exercise 8: Testing the SUT
Test your sut with the developed test cases, either manually, or using some existing or self-developed test execution tool. Describe for each test case the outcome of test execution.
## Exercise 9: Analysation and explanation
Analyze and explain the observed test results.
## Exercise 10: Test tools
If you used test tools or automated test execution, then please provide the code, in such a way that we can run it; provide a ’README’. Be prepared to give a demo.