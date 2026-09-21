# Test Preparation
## Exercise 1: Description of SUT
For our system-under-test we have picked the [Synapse client](https://github.com/element-hq/synapse) by [Element](https://element.io/), an open source instance of [Matrix](https://matrix.org/) written in Python.
- b) what functionality does your sut provide for its users;
     With this SUT the user is able to:
     - Locally run the synapse server.
     - Create a web-client on app.element.io.
     These components together enable communication based on chat messages following the matrix communication protocol.
- c) external perspective: functionality, what the sut shall do, its interfaces, the inputs it accepts, the outputs that can be observed, possible and necessary environments, how to start and stop the sut,. . . , including a picture of the external view;
     From the outside, the SUT lets a user send and receive chat messages according to the Matrix protocol.
     - Functionality: user registration and login, creating/joining/leaving rooms, sending and receiving messages, viewing message history.
     - Interfaces: the Matrix Client-Server HTTP API exposed by Synapse at `/_matrix/client/*` on `localhost:8008`, accessed either through `app.element.io` or directly via HTTP requests (curl).
     - Inputs: user credentials (registration/login), room actions (create, join, invite), and messages, entered through the `app.element.io` UI or sent as JSON in HTTP requests.
     - Outputs: rendered rooms and messages in the `app.element.io` UI, or raw JSON responses and HTTP status codes when interacting with the API directly.
     - Environment: Synapse runs in a Docker container built from source on the tester's machine; Element runs as a web client in a browser, connecting to the local Synapse instance over HTTP.
     - Start/stop: the server is started with `docker run` (built beforehand with `docker build`) and stopped with `docker stop`/`docker rm`; the client is simply opened/closed in a browser tab, with the homeserver URL pointed at `http://localhost:8008`.
- d) internal perspective: structure and implementation details as far as they matter for black-box testing, . . . , including a picture of the structural view;
     For black-box testing is the SUT's internal structure mainly the components which has a observable form of output:
     - HTTP Listener on port 8008 passing request based on content and path.
     - Client-server and API handler process requests in accordance with the Matrix communication protocol.
- e) the software/hardware platform(s) on which it is running, required additional software for running it, version number, configuration, and any other specific information about the sut;
     an docker container ...
- f) references to documentation about your sut.
  https://matrix.org/docs/
  https://hub.docker.com/r/matrixdotorg/synapse
  https://github.com/element-hq/synapse
## Exercise 2: What part of your SUT to test
- a) which part(s), components, features, functionality, interfaces;
- b) which requirements/specifications apply to these parts; refer to available documentation, where applicable.
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