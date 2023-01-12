////////////////////////////////////////////////////////////////
// ELEX 4618 Client Template project for BCIT
// Created Oct 5, 2016 by Craig Hennessey
// Last updated March 29, 2019
////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <string>
#include <iostream>
#include <thread>

#include "Client.h"

Client client(4618, "192.168.137.18");

void send_command(std::string command) {
	std::string response;
	client.tx_str(command);
	
	do
	{
		client.rx_str(response);
		if (response.length() > 0)
		{
			std::cout << "\nClient Rx: " << response;
		}
	} while (response.length() == 0);
}

void get_image()
{
	cv::Mat im;
	int count = 0;

    client.tx_str("im");

	if (client.rx_im(im) == true)
	{
		if (im.empty() == false)
		{
			count++;
			std::cout << "\nImage received: " << count;
			cv::imshow("rx", im);
			cv::waitKey(100);
		}
	}
}

void print_menu()
{
	std::cout << "\n***********************************";
	std::cout << "\n* ELEX4618 Recycling Sorter Network";
	std::cout << "\n***********************************";
	std::cout << "\n(1) Turn System On";
	std::cout << "\n(2) Turn System Off";
	std::cout << "\n(3) Send to BIN 1";
	std::cout << "\n(4) Send to BIN 2";
	std::cout << "\n(5) Get System Status";
	std::cout << "\n(6) Get BIN 1 count";
	std::cout << "\n(7) Get BIN 2 count";
	std::cout << "\n(8) Get Image";
	std::cout << "\n(0) Exit";
	std::cout << "\nCMD> ";
}


int main(int argc, char* argv[])
{
	int cmd = -1;
	std::thread t1;
	std::string command;
	bool video_live = false;

	do
	{
		print_menu();

		if (video_live) {
			get_image();
		}
		
		
		std::cin >> cmd;
		switch (cmd)
		{
		case 1: send_command("S 0 1"); break;
		case 2: send_command("S 0 0"); break;
		case 3: send_command("S 1 0"); break;
		case 4: send_command("S 1 1"); break;
		case 5: send_command("G 0 "); break;
		case 6: send_command("G 1 0"); break;
		case 7: send_command("G 1 1"); break;
		case 8: video_live = true;; break;
		}

	//	std::thread t1(&get_image);
	//	t1.detach();

	} while (cmd != 0);
}
