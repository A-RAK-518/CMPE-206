/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// Homework 1 Network Topology
//
//                     10.1.1.0
//n6----------- n0 -------------- n1   n2   n3   n4  n5
//    point-to-point               |   |    |    |   | 
//                                 ===================
//                                   LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
 
  
  uint32_t nCsma = 3; 
/*  uint32_t nCsma1 = 4;  */
  
  CommandLine cmd;
  
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  nCsma = nCsma == 0 ? 1 : nCsma;
 /*  nCsma1 = nCsma1 == 0 ? 1 : nCsma1; */

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  // Creating another Point to Point link to the leftmost corner of n0
  
  NodeContainer p2pNodes1;
  p2pNodes1.Create (2);

  NodeContainer csmaNodes;

  
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);
/*  
  csmaNodes.Add (p2pNodes1.Get (1));
  csmaNodes.Create (nCsma1);  
*/

  //the point to point link is being designed here, i will use the same parameters (delay and data rate ) for both the p2p links from n6 - n0 and from n0 - n1
  
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
 
  // Creating a new NetDevice For the new p2p link p2pNodes1 
  
  NetDeviceContainer p2pDevices1;
  
  p2pDevices = pointToPoint.Install (p2pNodes);
 
  // Istalling the NetDevices for the new p2p Link n6 - n0
  p2pDevices1 = pointToPoint.Install (p2pNodes1);

  // the csma lan data rate and the delay are being mentioned here
  
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (p2pNodes1.Get (0)); // installing the internet stack for p2p link n6 - n0
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  
  Ipv4InterfaceContainer p2pInterfaces;
  
  p2pInterfaces = address.Assign (p2pDevices);
  
  Ipv4InterfaceContainer p2pInterfaces1; // creating a new point to point interface to hold the IP addresses range for the link n6 - n0
  
  address.SetBase ("10.1.3.0", "255.255.255.0"); // Assigning a new IP address range for the new point to point link with the 10.1.3.0/24
  
  p2pInterfaces1 = address.Assign (p2pDevices1); // Assigning the addresses from the subnet 10.1.3.0/24 to the point to point interface links n6 - n0

  address.SetBase ("10.1.2.0", "255.255.255.0");

  Ipv4InterfaceContainer csmaInterfaces;
  
  csmaInterfaces = address.Assign (csmaDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  //  Creating a new echo server and asking it to use port 10 to listen for echo requests

  UdpEchoServiceHelper echoserver1 (10);
  ApplicationContainer serverApps1 = echoserver1.Install (csmaNodes.Get (nCsma));
  serverApps1.Start (Seconds (1.0));
  serverApps1.Stop (Seconds (10.0)); 

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

 //  Creating a new echo client and asking it to use port 10 to listen for echo replies
  
  UdpEchoClientHelper echoClient1 (csmaInterfaces.GetAddress (nCsma), 10);
  echoClient1.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

 //  Creating a new start and stop time for the new Echo client

  ApplicationContainer clientApps1 = echoclient1.Install (p2pNodes1.Get (0));
  clientApps1.Start (Seconds (2.0));
  clientApps1.Start (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  pointToPoint.EnablePcapAll ("second");
  csma.EnablePcap ("second", csmaDevices.Get (1), true);
 /*  csma.EnablePcap ("Third" ,csmaDevices1.Get (1), true); */

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
