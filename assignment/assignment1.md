# Test Preparation
## Exercise 1: Description of SUT
For our system-under-test we have picked [Synapse](https://github.com/element-hq/synapse) a (home)server developed by [Element](https://element.io/). Synapse is an open source instance of [Matrix](https://matrix.org/) written in Python.

Using Synapse, the user can locally run the a server and connect to it using a web client on [app.element.io](https://app.element.io/). These enable chat-based communication following the Matrix communication protocol.

As a chat server, Synapse allows a user to communicate in accordance with the Matrix protocol. 
- Functionality includes user registration and login, creating/joining/leaving rooms, sending and receiving messages and viewing message history. 
- The interface is the Matrix Client-Server HTTP API exposed by Synapse at `/_matrix/client/*` on `localhost:8008`, accessed either through `app.element.io` or directly via HTTP requests.
- The inputs include user credentials (registration/login), room actions (create, join, invite), and messages, entered through the `app.element.io` GUI or sent as JSON in HTTP requests.
- The outputs consist of rooms and messages rendered in the `app.element.io` UI, or viewed as raw JSON responses and HTTP status codes when interacting with the API directly.
- Synapse runs in a Docker container built from source on the tester's machine. Element runs as a web client in a browser, connecting to the local Synapse instance over HTTP.
- The server is started with `docker run` (built beforehand with `docker build`) and stopped with `docker stop`/`docker rm`; the client is simply opened/closed in a browser tab, with the homeserver URL matching that of port exposed by Docker (see [[TestingTechniques1/README]]).
==TODO: add picture of external view.==

For black-box testing is the SUT's internal structure mainly the components which has a observable form of output:
- HTTP Listener on port `8008` passing request based on content and path.
- Client-server and API handler process requests in accordance with the Matrix communication protocol.
==TODO: add picture of internal view.==

- e) the software/hardware platform(s) on which it is running, required additional software for running it, version number, configuration, and any other specific information about the sut;
     - a docker container running the [latest image version](https://hub.docker.com/r/matrixdotorg/synapse).
     - synapse repository also on the latest ([1.161.0](https://github.com/element-hq/synapse/releases/tag/v1.161.0) ) version
     - The repository and image are both run locally on either linux or windows
- f) references to documentation about your sut.
  https://matrix.org/docs/
  https://hub.docker.com/r/matrixdotorg/synapse
  https://github.com/element-hq/synapse
  https://spec.matrix.org/latest/
## Exercise 2: What part of your SUT to test
We're going to test chatting between two users in chatrooms. This way, we won't need to reset the container between runs. Some basic test cases we thought of are:
- Sending a message
- Deleting a message
- Adding a reaction to a message
- Removing a reaction
- Replying to a message
- Pinning a message


- a) which part(s), components, features, functionality, interfaces;
   We intend to test the part dealing with chatting in a room primairly from the perspective of the user interaction that happens on the server.
   This means we need to test the components dealing with client-server api and room events partially.
   We intend to test the following features/functionality: sending a message, deleting a message, replying to a message, removing a reply. 
   This testing will be done trough the client-server api as interface.
  
- b) which requirements/specifications apply to these parts; refer to available documentation, where applicable.
   This is all found in the matrix specification: https://spec.matrix.org/v1.19/client-server-api/
   To be precise see the following table
   |feature/part| chapters|
   |------------| --------|
   | room events| 7       |
   | sending a message | 10.2|
   | deleting a message | 7.9|
   | replying to a message | 10.3|
   | removing a reply| 7.9 |

   note that sending a message and replying to it are both events and thus both are redacted the same way.
## Exercise 3: Test architecture for the testing
Provide a test architecture for the testing you are going to perform:
You can choose whether you do manual testing, i.e., a human gives inputs
to and observes outputs from the sut, or automated testing, i.e., some
program, test tool, or test script gives inputs to the sut and observes its
outputs.
- a) give a diagram in which you position the sut, its (distributed) structure, interfaces, environment, stubs, drivers, necessary test tools, . . . ;
- b) describe the sut input interfaces that you will use, i.e., where and how will your tests will trigger the sut;
- c) describe the sut output interfaces, i.e., where and what you will observe during testing;
- d) describe assumptions on the environment of the sut;
- e) describe which tools you will use for testing, consistent with the diagram that you made above. Tools can be stubs, drivers, mocks, open-source test tools (search the internet), self-developed test tools, shell scripts, interface tools, protocol sniffers, . . ..
## Exercise 4: What typical test cases look like
- a) what is a typical structure of your tests: initial state, test inputs, test outputs, observations, conditions, . . . .
- b) how will you specify your test cases, i.e., the test notation or language to document your test cases; give a template for a test case.
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