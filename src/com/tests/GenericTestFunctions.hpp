#pragma once

using namespace precice;

/// Generic test function that is called from the tests for MPIPortsCommunication,
/// MPIPortsCommunication and SocketCommunication


template<typename T>
void TestSendAndReceivePrimitiveTypes()
{
  T com;

  if (utils::Parallel::getProcessRank() == 0) {
    com.acceptConnection("process0", "process1");
    {
      std::string msg("testOne");
      com.send(msg, 0);
      com.receive(msg, 0);
      BOOST_TEST(msg == std::string("testTwo"));
    }
    {
      double msg = 0.0;
      com.send(msg, 0);
      com.receive(msg, 0);
      BOOST_TEST(msg == 1.0);
    }
    {
      int msg = 1;
      com.send(msg, 0);
      com.receive(msg, 0);
      BOOST_TEST(msg == 2);
    }
    {
      bool msg = true;
      com.send(msg, 0);
      com.receive(msg, 0);
      BOOST_TEST(msg == false);
    }
    com.closeConnection();
  } else if (utils::Parallel::getProcessRank() == 1) {
    com.requestConnection("process0", "process1", 0, 1);
    {
      std::string msg;
      com.receive(msg, 0);
      BOOST_TEST(msg == std::string("testOne"));
      msg = "testTwo";
      com.send(msg, 0);
    }
    {
      double msg = 1.0;
      com.receive(msg, 0);
      BOOST_TEST(msg == 0.0);
      msg = 1.0;
      com.send(msg, 0);
    }
    {
      int msg = 0;
      com.receive(msg, 0);
      BOOST_TEST(msg == 1);
      msg = 2;
      com.send(msg, 0);
    }
    {
      bool msg = false;
      com.receive(msg, 0);
      BOOST_TEST(msg == true);
      msg = false;
      com.send(msg, 0);
    }
    com.closeConnection();
  }


}

template<typename T>
void TestSendAndReceiveVectors()
{
  T com;
  if (utils::Parallel::getProcessRank() == 0) {
    com.acceptConnection("process0", "process1");
     {
      Eigen::Vector3d msg = Eigen::Vector3d::Constant(0);
      com.receive(msg.data(), msg.size(), 0);
      BOOST_CHECK(testing::equals(msg, Eigen::Vector3d::Constant(1)));
      msg = Eigen::Vector3d::Constant(2);
      com.send(msg.data(), msg.size(), 0);
    }
    {
      Eigen::Vector4i msg = Eigen::Vector4i::Constant(0);
      com.receive(msg.data(), msg.size(), 0);
      BOOST_CHECK(testing::equals(msg, Eigen::Vector4i::Constant(1)));
      msg = Eigen::Vector4i::Constant(0);
      com.send(msg.data(), msg.size(), 0);
    }
    {
      std::vector<int> msg;
      std::vector<int> recv{1, 2, 3};
      com.receive(msg, 0);
      BOOST_TEST(msg == recv);
      com.send(msg, 0);
    }
    {
      std::vector<double> msg;
      com.receive(msg, 0);
      BOOST_TEST(msg == std::vector<double>({1.1, 2.2, 3.3}));
      com.send(msg, 0);
    }
    com.closeConnection();
  } else if (utils::Parallel::getProcessRank() == 1) {
    com.requestConnection("process0", "process1", 0, 1);
    {
      Eigen::Vector3d msg = Eigen::Vector3d::Constant(1);
      com.send(msg.data(), msg.size(), 0);
      com.receive(msg.data(), msg.size(), 0);
      BOOST_CHECK(testing::equals(msg, Eigen::Vector3d::Constant(2)));
    }
    {
      Eigen::Vector4i msg = Eigen::Vector4i::Constant(1);
      com.send(msg.data(), msg.size(), 0);
      com.receive(msg.data(), msg.size(), 0);
      BOOST_CHECK(testing::equals(msg, Eigen::Vector4i::Zero()));
    }
    {
      std::vector<int> msg{1, 2, 3};
      com.send(msg, 0);
      com.receive(msg, 0);
      BOOST_CHECK(msg == std::vector<int>({1, 2, 3}));
    }
    {
      std::vector<double> msg{1.1, 2.2, 3.3};
      com.send(msg, 0);
      com.receive(msg, 0);
      BOOST_CHECK(msg == std::vector<double>({1.1, 2.2, 3.3}));
    }
    com.closeConnection();
  }
}

template<typename T>
void TestSendAndReceive()
{
  TestSendAndReceivePrimitiveTypes<T>();
  TestSendAndReceiveVectors<T>();
}

template<typename T>
void TestSendReceiveTwoProcessesServerClient()
{
  T communication;

  std::string nameEven("even");
  std::string nameOdd("odd");

  switch (utils::Parallel::getProcessRank()) {
  case 0: {
    communication.acceptConnectionAsServer(nameEven, nameOdd, 1);
    int message = 1;
    communication.send(message, 0);
    communication.receive(message, 0);
    BOOST_TEST(message == 2);
    communication.closeConnection();
    break;
  }
  case 1: {
    communication.requestConnectionAsClient(nameEven, nameOdd);
    int message = -1;
    communication.receive(message, 0);
    BOOST_TEST(message == 1);
    message = 2;
    communication.send(message, 0);
    communication.closeConnection();
    break;
  }
  }
}

template<typename T>
void TestSendReceiveFourProcessesServerClient()
{
  T communication;

  std::string nameEven("even");
  std::string nameOdd("odd");

  switch (utils::Parallel::getProcessRank()) {
  case 0: {
    communication.acceptConnectionAsServer(nameEven, nameOdd, 2);

    int requestorLocalRank = 0;
    int requestorGlobalRank = -1;
    communication.receive(requestorGlobalRank, requestorLocalRank);
    BOOST_TEST(requestorGlobalRank >= 2);
    BOOST_TEST(requestorGlobalRank <= 3);
    int message = requestorGlobalRank * 10;
    communication.send(message, requestorLocalRank);
    communication.receive(message, requestorLocalRank);
    BOOST_TEST(message == requestorGlobalRank * 10 + 2);

    requestorLocalRank = 1;
    requestorGlobalRank = -1;
    communication.receive(requestorGlobalRank, requestorLocalRank);
    BOOST_TEST(requestorGlobalRank >= 2);
    BOOST_TEST(requestorGlobalRank <= 3);
    message = requestorGlobalRank * 10;
    communication.send(message, requestorLocalRank);
    communication.receive(message, requestorLocalRank);
    BOOST_TEST(message == requestorGlobalRank * 10 + 2);

    communication.closeConnection();
    break;
  }
  case 1: {
    // does not accept a connection
    break;
  }
  case 2: {
    communication.requestConnectionAsClient(nameEven, nameOdd);
    int globalRank = 2;
    communication.send(globalRank,0);

    int message = -1;
    communication.receive(message, 0);
    BOOST_TEST(message == 20);
    message += 2;
    communication.send(message, 0);

    communication.closeConnection();
    break;
  }
  case 3: {
    communication.requestConnectionAsClient(nameEven, nameOdd);
    int globalRank = 3;
    communication.send(globalRank,0);

    int message = -1;
    communication.receive(message, 0);
    BOOST_TEST(message == 30);
    message += 2;
    communication.send(message, 0);

    communication.closeConnection();
    break;
  }
  }
}
