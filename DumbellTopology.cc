
#include <string>
#include <cstdlib>
#include <map>
#include <fstream>
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/gnuplot.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"


using namespace ns3;




/*

// Network topology
//
//   n0                                               n4
//     \ 80 Mb/s, 20ms 					            /   80 Mb/s, 20ms
//      \                 30Mb/s, 100ms            /
//       n2---------------------------------------n3
//      /					 		   				\
//     / 80 Mb/s, 20ms                   			 \	80 Mb/s, 20ms
//   n1 								 			  n5

*/



int main (int argc, char *argv[])
{

	// packet sizes 

	int ucp_packet_size=0;
	int tcp_packet_size=0;
	bool simultaneously = false;

	//Variales for Graph data set

	
	Gnuplot2dDataset data_tcp_throughput;
	Gnuplot2dDataset data_tcp_delay;
    Gnuplot2dDataset data_udp_delay;
    Gnuplot2dDataset data_udp_throughput;

	double delay_tcp;
	double delay_udp;

	double throughput_tcp;
    double throughput_udp;
    
	

	

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpHighSpeed::GetTypeId ()));

	for(int i=0;i<30; ++i)
	{		
		  

		//create nodes 
		NodeContainer nodes;
		nodes.Create(6);

		NodeContainer h1r1 = NodeContainer(nodes.Get(0),nodes.Get(2));
		NodeContainer h2r1 = NodeContainer(nodes.Get(1),nodes.Get(2));
		NodeContainer r1r2 = NodeContainer(nodes.Get(2),nodes.Get(3));
		NodeContainer h3r2 = NodeContainer(nodes.Get(4),nodes.Get(3));
		NodeContainer h4r2 = NodeContainer(nodes.Get(5),nodes.Get(3));
		
		PointToPointHelper pTpHelper;

		pTpHelper.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("375000B")); // DropTailQueue
		pTpHelper.SetDeviceAttribute("DataRate",StringValue("30Mbps"));
		pTpHelper.SetChannelAttribute("Delay",StringValue("100ms"));

		NetDeviceContainer d2_d3;
		d2_d3 = pTpHelper.Install(r1r2);


		pTpHelper.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("200000B")); 
		pTpHelper.SetDeviceAttribute("DataRate",StringValue("80Mbps"));
		pTpHelper.SetChannelAttribute("Delay",StringValue("20ms"));

		NetDeviceContainer d0_d2=pTpHelper.Install(h1r1);
		NetDeviceContainer d1_d2=pTpHelper.Install(h2r1);
		NetDeviceContainer d4_d3=pTpHelper.Install(h3r2);
		NetDeviceContainer d5_d3=pTpHelper.Install(h4r2);

		InternetStackHelper internet_prots;
		internet_prots.Install (nodes);

		Ipv4AddressHelper add;
		add.SetBase("10.1.1.0","255.255.255.0");

		Ipv4InterfaceContainer ip_i0_i21=add.Assign(d0_d2);

		add.SetBase("10.1.2.0","255.255.255.0");

		Ipv4InterfaceContainer ip_i1_i22=add.Assign(d1_d2);

		add.SetBase("10.1.3.0","255.255.255.0");

		Ipv4InterfaceContainer ip_i2_i3=add.Assign(d2_d3);

		add.SetBase("10.1.4.0","255.255.255.0");

		Ipv4InterfaceContainer ip_i4_i31=add.Assign(d4_d3);

		add.SetBase("10.1.5.0","255.255.255.0");

		Ipv4InterfaceContainer ip_i5_i32=add.Assign(d5_d3);

		Ptr<RateErrorModel> ErrModel = CreateObject<RateErrorModel> ();
	    ErrModel->SetAttribute ("ErrorRate", DoubleValue (0.000001));
	    d4_d3.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (ErrModel));
	    d5_d3.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (ErrModel));
		

		// Allowing Global routing 
		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


		
		Address udp_dest_add = (InetSocketAddress (ip_i4_i31.GetAddress(0), 1998));
		OnOffHelper udpCliet ("ns3::UdpSocketFactory", udp_dest_add);

		Address udp_sink_add =(InetSocketAddress (Ipv4Address::GetAny (), 1998)); 
		PacketSinkHelper sink_udp ("ns3::UdpSocketFactory",udp_sink_add);


		ucp_packet_size= ucp_packet_size + 100;

		ApplicationContainer udp_sink_app = sink_udp.Install (h3r2.Get(0));
	
	    udpCliet.SetAttribute ("PacketSize", UintegerValue (ucp_packet_size));

		ApplicationContainer cbr_App = udpCliet.Install (h1r1.Get(0));




		Address tcp_sink_add = InetSocketAddress (Ipv4Address::GetAny (), 2001);
	    PacketSinkHelper sink_tcp ("ns3::TcpSocketFactory",tcp_sink_add);
	    ApplicationContainer tcp_sink_app = sink_tcp.Install (h4r2.Get(0));

		Address tcp_dest_add = InetSocketAddress (ip_i5_i32.GetAddress(0), 2001);
	    BulkSendHelper ftp_helper ("ns3::TcpSocketFactory", tcp_dest_add );

	    tcp_packet_size= tcp_packet_size + 100;
	    ftp_helper.SetAttribute ("SendSize", UintegerValue(tcp_packet_size));
	    ftp_helper.SetAttribute ("MaxBytes", UintegerValue (0));
	    ApplicationContainer FTP_App = ftp_helper.Install (h2r1.Get(0));


		
		if(simultaneously==false)
		{
			udp_sink_app.Start (Seconds ((0.0+(16*i))) );
			udp_sink_app.Stop (Seconds ((8.0+(16*i))) );
			cbr_App.Start (Seconds ((0.0+(16*i)) ) );
			cbr_App.Stop (Seconds ((8.0+(16* i))) );
			tcp_sink_app.Start (Seconds ((8.0+(16*i))) );
	    	tcp_sink_app.Stop (Seconds ((16.0+(16*i))) );
			FTP_App.Start (Seconds ((8.0+(16*i))) );
	    	FTP_App.Stop (Seconds ((16.0+(16*i))) );	

		}
		else
		{
			udp_sink_app.Start (Seconds ((0.0+(16*i))) );
			udp_sink_app.Stop (Seconds ((16.0+(16*i))) );
			cbr_App.Start (Seconds ( (0.0+(16*i)) ) );
			cbr_App.Stop (Seconds ((16.0+(16*i))) );
			tcp_sink_app.Start (Seconds ((0.0+(16*i))) );
	    	tcp_sink_app.Stop (Seconds ((16.0+(16*i))) );
			FTP_App.Start (Seconds ((0.0+(16*i))) );
	    	FTP_App.Stop (Seconds ((16.0+(16*i))) );
			
		}


		Ptr<FlowMonitor> f_monitor;
		FlowMonitorHelper flow_helper;
		f_monitor = flow_helper.InstallAll();
		Simulator::Stop(Seconds((16+(16*i))) );
		Simulator::Run();
		f_monitor->CheckForLostPackets();

		Ptr<Ipv4FlowClassifier> clfr = DynamicCast<Ipv4FlowClassifier>(flow_helper.GetClassifier());
		std::map<FlowId, FlowMonitor::FlowStats> flow_stats = f_monitor->GetFlowStats();
		std::map<FlowId, FlowMonitor::FlowStats>::const_iterator it = flow_stats.begin();
		
		while( it != flow_stats.end()) 
		{
			Ipv4FlowClassifier::FiveTuple tup = clfr->FindFlow (it->first);
			std::cout<<"S_ADD "<<tup.sourceAddress<<std::endl;
			std::cout<<"D_ADD "<<tup.destinationAddress<<std::endl;

			
			if(tup.sourceAddress == "10.1.2.1") {
				throughput_tcp = it->second.rxBytes * 8.0 / (it->second.timeLastRxPacket.GetSeconds () - it->second.timeFirstTxPacket.GetSeconds ()) / 1000;
				delay_tcp = (it->second.delaySum.GetSeconds()/(it->second.rxPackets))*1000;
				std::cout<<"PacketSizes "<<tcp_packet_size<<" Tcp Delay "<<delay_tcp<<std::endl;
				std::cout<<"PacketSizes "<<tcp_packet_size<<" Tcp Throughput "<<throughput_tcp<<std::endl;
				
				data_tcp_throughput.Add (tcp_packet_size,throughput_tcp);
				data_tcp_delay.Add(tcp_packet_size,delay_tcp);

			}

			else if(tup.sourceAddress == "10.1.1.1") {
				throughput_udp = it->second.rxBytes * 8.0 / (it->second.timeLastRxPacket.GetSeconds () - it->second.timeFirstTxPacket.GetSeconds ()) / 1000;
				delay_udp = (it->second.delaySum.GetSeconds()/(it->second.rxPackets))*1000 ;

				std::cout<<"PacketSizes "<<ucp_packet_size<<" Udp Delay "<<delay_udp<<std::endl;
				std::cout<<"PacketSizes "<<ucp_packet_size<<" Udp Throughput "<<throughput_udp<<std::endl;


				data_udp_throughput.Add (ucp_packet_size,throughput_udp);
				data_udp_delay.Add (ucp_packet_size,delay_udp);

				

			} 

			it++;
		}
		
		Simulator::Destroy ();
	}

		std::string plot_heading = "TcpHighSpeed Vs UDP Throughput";
		std::string plot_file_name_noEX = "Plot";
		std::string plot_file_name = plot_file_name_noEX + ".plt";

		std::string graphic_File_name_NoEx = "Graphic";
		std::string graphic_File_name = graphic_File_name_NoEx + ".png";

		std::string plot_heading_Delay = "TcpHighSpeed Vs UDP Delay";
		std::string plot_file_name_Delay_noEX = "Plot_Delay";
		std::string plot_file_name_Delay = plot_file_name_Delay_noEX + ".plt";

		std::string graphic_File_name_Delay_noEX = "Graphic_Delay";
		std::string graphic_File_name_Delay = graphic_File_name_Delay_noEX + ".png";

	

	Gnuplot Throughput_Plot (graphic_File_name);
	Gnuplot Delay_Plot (graphic_File_name_Delay);
	Throughput_Plot.SetTitle(plot_heading);
	Delay_Plot.SetTitle(plot_heading_Delay);
	Throughput_Plot.SetTerminal("png");
	Delay_Plot.SetTerminal("png");
	Throughput_Plot.SetLegend("PacketSize(Bytes)","Throughput(Kbps)");
	Delay_Plot.SetLegend("PacketSize(Bytes)","Delay(ms)");
	data_tcp_throughput.SetTitle("Throughput FTP over TcpHighSpeed");
	data_udp_delay.SetTitle("Delay CBR Over UDP");
	data_udp_throughput.SetTitle("Throughput CBR Over UDP");
	data_tcp_delay.SetTitle("Delay FTP over TcpHighSpeed");

	data_tcp_throughput.SetStyle(Gnuplot2dDataset::LINES_POINTS);
	data_udp_throughput.SetStyle(Gnuplot2dDataset::LINES_POINTS);
	data_tcp_delay.SetStyle(Gnuplot2dDataset::LINES_POINTS);
	data_udp_delay.SetStyle(Gnuplot2dDataset::LINES_POINTS);


	Throughput_Plot.AddDataset(data_tcp_throughput);
	Throughput_Plot.AddDataset(data_udp_throughput);
	Delay_Plot.AddDataset(data_tcp_delay);
	Delay_Plot.AddDataset(data_udp_delay);

	std::ofstream plotFile_delay(plot_file_name_Delay.c_str());
	Delay_Plot.GenerateOutput(plotFile_delay);
	plotFile_delay.close();

	std::ofstream plotFile(plot_file_name.c_str());
	Throughput_Plot.GenerateOutput(plotFile);
	plotFile.close();

	

	return 0;
}	