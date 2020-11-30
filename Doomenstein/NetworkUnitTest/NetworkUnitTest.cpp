#include "pch.h"
#include "CppUnitTest.h"

#include "Engine/Network/UDPSocket.hpp"
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <array>
#include <string>
#include <thread>
#include <tuple>
#include <mutex>
#include <iostream>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NetworkUnitTest
{
	TEST_CLASS(SocketTestCase)
	{
	public:

		TEST_METHOD_INITIALIZE(InitUDPSocketTest)
		{
			WSADATA wsaData;
			WORD wVersion MAKEWORD(2, 2);
			int iResult = WSAStartup(wVersion, &wsaData);
			if (iResult != 0)
			{
				std::cout << "Networking System: WSAStartup failed with " << WSAGetLastError() << std::endl;
			}
		}

		struct MessageHeader
		{
			uint16_t id;
			uint16_t length;
			uint16_t seqNo;
		};

		using TextArray = std::array<std::string, 5>;
		TextArray text = {
			"Hello",
			"It is a very nice day outside",
			"Watch out!!!!",
			"We have reached the end",
			"Goodbye!",
		};

		uint16_t toU16(size_t input) { return static_cast<uint16_t>(input); };

		using MessageArray = std::array<std::tuple<unsigned short, unsigned short, unsigned short, std::string>, 5>;
		MessageArray messages = {
			std::make_tuple(1, 0, toU16(strlen(text[0].c_str())), text[0]),
			std::make_tuple(1, 1, toU16(strlen(text[1].c_str())), text[1]),
			std::make_tuple(1, 2, toU16(strlen(text[2].c_str())), text[2]),
			std::make_tuple(1, 3, toU16(strlen(text[3].c_str())), text[3]),
			std::make_tuple(0, 4, toU16(strlen(text[4].c_str())), text[4]),
		};

		static void writer(UDPSocket& socket, MessageArray const& messages)
		{
			MessageHeader header;
			for (auto msg : messages)
			{
				header.id = std::get<0>(msg);
				header.length = std::get<2>(msg);
				header.seqNo = std::get<1>(msg);

				auto& buffer = socket.SendBuffer();
				*reinterpret_cast<MessageHeader*>(&buffer[0]) = header;

				memcpy(&(socket.SendBuffer()[sizeof(MessageHeader)]), std::get<3>(msg).c_str(), header.length);

				socket.SendBuffer()[sizeof(MessageHeader) + header.length] = NULL;

				socket.Send(sizeof(MessageHeader) + header.length + 1);
			}
		}

		static void reader(UDPSocket& socket, TextArray& messages)
		{
			static std::mutex lock;

			MessageHeader const* pMsg = nullptr;
			std::string dataStr;
			size_t length = 0;

			do
			{
				length = socket.Receive();

				dataStr.clear();
				if (length > 0)
				{
					auto& buffer = socket.ReceiveBuffer();
					pMsg = reinterpret_cast<MessageHeader const*>(&buffer[0]);
					if (pMsg->length > 0)
					{
						dataStr = &buffer[sizeof(MessageHeader)];
					}
					messages[pMsg->seqNo] = dataStr;
					{
						std::lock_guard<std::mutex> guard(lock);

						std::ostringstream ostr;
						ostr << "Thread [" << std::this_thread::get_id() << "] received message"
							<< "  id = " << pMsg->id
							<< "  size = " << pMsg->length
							<< "  seqNo = " << pMsg->seqNo
							<< "  data = " << dataStr
							<< std::endl << std::ends;
						Logger::WriteMessage(ostr.str().c_str());
					}
				}
			} while (pMsg != nullptr && pMsg->id != 0);
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(UDPSocketTest)
			TEST_DESCRIPTION(L"Test simultaneous reading and writing on UDP sockets")
		END_TEST_METHOD_ATTRIBUTE()


		TEST_METHOD(UDPSocketTest)
		{
// 			Logger::WriteMessage("Starting UDP Socket Test");
// 
// 			UDPSocket socketOne("127.0.0.1", 48000);
// 			socketOne.Bind(48001);
// 
// 			UDPSocket socketTwo("127.0.0.1", 48001);
// 			socketTwo.Bind(48000);
// 
// 			TextArray readMessageOne;
// 			TextArray readMessageTwo;
// 
// 			std::thread writerOne(&SocketTestCase::writer, std::ref(socketOne), std::cref(messages));
// 
// 			std::thread readerOne(&SocketTestCase::reader, std::ref(socketOne), std::ref(readMessageOne));
// 			std::thread readerTwo(&SocketTestCase::reader, std::ref(socketTwo), std::ref(readMessageTwo));
// 
// 			std::thread writerTwo(&SocketTestCase::writer, std::ref(socketTwo), std::cref(messages));
// 
// 			writerOne.join();
// 			writerTwo.join();
// 			readerOne.join();
// 			readerTwo.join();
// 
// 			Assert::IsTrue(text == readMessageOne);
// 			Assert::IsTrue(text == readMessageTwo);
		}

		TEST_METHOD_CLEANUP(DeInitUDPSocketTest)
		{
			int iResult = WSACleanup();
			if (iResult == SOCKET_ERROR)
			{
				std::cout << "Networking System:: WSACleanup failed with " << WSAGetLastError() << std::endl;
			}
		}
	};
}
