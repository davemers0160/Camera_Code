using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.IO.Ports;
using System.Globalization;
using Microsoft.Win32;

using Packet_Class;

namespace Lens_Driver_GUI
{
    public partial class Form1 : Form
    {

        bool LED_Status = false;                            // track LED status
        public bool Connected = false;                      // track the connections status
        public SerialPort serialPort1 = new SerialPort();   // create serial port entity
        private byte amplitude;

        // Form1 initialization routine
        public Form1()
        {
            InitializeComponent();
            foreach (string ports in SerialPort.GetPortNames())     // cycle through port names to list in combo box
            {
                port_combo.Items.Add(ports);
            }
            port_combo.Sorted = true;                       // sort in ascending order
            port_combo.SelectedIndex = port_combo.Items.Count-1;
            baudRateBox.SelectedIndex = 7;                  // set default value in baud rate combobox
            volt_GB.Enabled = false;

        }


        /* Function: private void connect_btn_Click(object sender, EventArgs e)
         *
         * Return Value:
         * 1. None
         *
         * Description: This function is used to open up a serial port based on the input from the front panel and 
         * then check to see if the user has connected to the microcontroller.*/
        private void connect_btn_Click(object sender, EventArgs e)
        {
            // check to make sure that the com port is closed, connected is false and the como box is not empty
            if (serialPort1.IsOpen == false & Connected == false & port_combo.Text != "")
            {
                // setup paramters for serial port connection
                serialPort1.BaudRate = Convert.ToInt32(baudRateBox.SelectedItem);  //115200;
                serialPort1.Parity = Parity.None;
                serialPort1.DataBits = 8;
                serialPort1.StopBits = StopBits.Two;
                serialPort1.PortName = port_combo.Text;

                serialPort1.Open();                         // open up serial port connection
                
                System.Threading.Thread.Sleep(10);          // put the thread to sleep for 10ms

                if (serialPort1.IsOpen == true)
                {
                    serialPort1.DiscardInBuffer();          // clear out serial port receive buffer

                    Packet Connect_PacketIn = new Packet();         // create received data packet
                    Packet Connect_Packet = new Packet(0x40);       // create transmit data packet

                    Connect_Packet.sendPacket(Connect_Packet, serialPort1); // send packet using specifiied serial port

                    Connect_PacketIn = readPort(serialPort1);   // read data in from serial port into Packet structure

                    // check for the correct return byte header and that the checksum is valid
                    if (Connect_PacketIn.command == 0x40 & Connect_PacketIn.checksum_valid == true)
                    {
                        status_PIC_SN.Text = Connect_PacketIn.data[0].ToString();

                        status_PIC_FW.Text = Connect_PacketIn.data[1].ToString() + "." + Connect_PacketIn.data[2].ToString("D2");

                        switch (Connect_PacketIn.data[3])
                        {
                            case 0:
                                status_DRV_TYP.Text = "Microchip HV892";
                                break;
                            case 1:
                                status_DRV_TYP.Text = "Maxim MAX15474";
                                break;
                            default:
                                break;
                        }


                        voltage_TB.Text = (((byte)voltage_Number.Value * 0.205) + 9.8).ToString("N3");


                        //if (Connect_PacketIn.data[0] == (byte)'!')  // check fo correct data byte
                        //{
                            connect_btn.Text = "Disconnect";        // change button text
                            Connected = true;                       // set connected flag
                            // write status to text console
                            textConsole.AppendText("m> Successfully connected!" + Environment.NewLine);
                       //     getSerialNumber(sender, e);
                        //    getFirmwareVersion(sender,e);
                        //    getDriverType(sender, e);
                            volt_GB.Enabled = true;

                            voltage_Number.Value = Connect_PacketIn.data[4];
                            voltage_TB.Text = ((Connect_PacketIn.data[4] * 0.205) + 9.8).ToString("N3");


                        //}
                        //else
                        //{
                        //    Connected = false;                      
                        //    connect_btn.Text = "Connect";
                        //    serialPort1.Close();
                        //    textConsole.AppendText("m> Could not connect!" + Environment.NewLine);
                        //    volt_GB.Enabled = false;
                        //}
                    }
                    // checksum of return header byte were not correct
                    else
                    {
                        Connected = false;
                        connect_btn.Text = "Connect";
                        serialPort1.Close();
                        textConsole.AppendText("m> Could not connect!" + Environment.NewLine);
                        volt_GB.Enabled = false;
                    }
                }
                // serial port is not open 
                else
                {
                    Connected = false;
                    connect_btn.Text = "Connect";
                    serialPort1.Close();
                    textConsole.AppendText("m> Could not connect!" + Environment.NewLine);
                    volt_GB.Enabled = false;
                }                
            }
            // serial port was open, connected was true or combo box was empty
            else if(connect_btn.Text == "Disconnect")
            {
                serialPort1.Close();
                Connected = false;
                connect_btn.Text = "Connect";
                textConsole.AppendText("m> Disconnected!" + Environment.NewLine);
                status_PIC_SN.Text = "";
                status_PIC_FW.Text = "";
                status_DRV_TYP.Text = "";
                volt_GB.Enabled = false;
            }
        }   // end of connect_btn_Click


        /* Function: private void port_combo_DropDown(object sender, EventArgs e)
         *
         * Return Value:
         * 1. None
         *
         * Description: This function fires off when the user clicks the drop down arrow on the combo box on the
         * GUI.  The function updates the combo box with available serial ports. */
        private void port_combo_DropDown(object sender, EventArgs e)
        {
            port_combo.Items.Clear();
            foreach (string ports in SerialPort.GetPortNames())
            {
                port_combo.Items.Add(ports);
            }
            port_combo.Sorted = true;
        }   // end of port_combo_DropDown


        /* Function: private void LED_Click(object sender, EventArgs e)
         *
         * Return Value:
         * 1. None
         *
         * Description: This function handles the click event when the user clicks on the LED graphic
         * on the front panel.  The LED is not a traditional control, but an image that can be made 
         * to act like a control. */
        private void LED_Click(object sender, EventArgs e)
        {
            if (Connected == true)                              // check connected status
            {
                serialPort1.DiscardInBuffer();
                Packet LED_Packet;
                Packet LED_PacketIn = new Packet();
                byte byte_count = 1;
                byte[] data = new byte[byte_count];             // initialize data array to the size of byte_count

                if (LED_Status == false)
                {
                    data[0] = 1;
                    LED_Packet = new Packet(0x20, byte_count, data);    // build packet 
                }
                else
                {
                    data[0] = 0;
                    LED_Packet = new Packet(0x20, byte_count, data);    // build packet
                }
                LED_Packet.sendPacket(LED_Packet, serialPort1);         // send packet to microcontroller

                LED_PacketIn = readPort(serialPort1);                   // read data in from serial port into packet

                // check for correct return header and valid checksum
                if(LED_PacketIn.command == 0x20 & LED_PacketIn.checksum_valid == true)
                {
                    if(LED_PacketIn.data[0] == 0)
                    {
                        LED.Image = Properties.Resources.LED_Off;   // set LED image to off image
                        LED_Status = false;                         // set LED status
                    }
                    else if (LED_PacketIn.data[0] == 1)
                    {
                        LED.Image = Properties.Resources.LED_On;    // set LED image to on image
                        LED_Status = true;                          // set LED status
                    }
                }               
            }
            else
            {
                // display message on text console to connect to PIC first
                textConsole.AppendText("e> Connect to PIC first." + Environment.NewLine);
            }
        }  // end of LED_Click


        private void getDriverType(object sender, EventArgs e)
        {
            if (Connected == true)
            {
                serialPort1.DiscardInBuffer();                      // clear serial port buffer
                Packet DT_PacketIn = new Packet();                  // create input packet structure

                Packet DT_Packet = new Packet(0x43);                // create packet
                DT_Packet.sendPacket(DT_Packet, serialPort1);       // send packet to microcontroller

                DT_PacketIn = readPort(serialPort1);                // read data in from serial port

                // check for correct return header and valid checksum
                if (DT_PacketIn.command == 0x43 & DT_PacketIn.checksum_valid == true)
                {
                    string driver_type = "";
                    // create a string with the received serial number
                    switch(DT_PacketIn.data[0])
                    {
                        case 0:
                            driver_type = "Microchip HV892";
                            break;
                        case 1:
                            driver_type = "Maxim MAX15474";
                            break;
                        case 2:
                            break;

                        default:
                            break;

                    }
                    status_DRV_TYP.Text = driver_type;
                    //string messageBoxText = "Serial Number: " + DT_PacketIn.data[0].ToString();
                    //string caption = "";    // create a blank string
                    // open a message box with the OK button and the string with the serial number
                    //MessageBox.Show(messageBoxText, caption, MessageBoxButtons.OK);
                }
            }
            else
            {
                // display message on text console to connect to PIC first
                textConsole.AppendText("e> Connect to PIC first." + Environment.NewLine);
            }
        }   // end of getDriverType


        /* Function: private void getSerialNumberToolStripMenuItem_Click(object sender, EventArgs e)
         *
         * Return Value:
         * 1. None
         *
         * Description: This function gets the serial number from the microcontroller and displays it in a 
         * separate message box.*/
        private void getSerialNumber(object sender, EventArgs e)
        {
            if (Connected == true)
            {
                serialPort1.DiscardInBuffer();                      // clear serial port buffer
                Packet SN_PacketIn = new Packet();                  // create input packet structure

                Packet SN_Packet = new Packet(0x42);                // create packet
                SN_Packet.sendPacket(SN_Packet, serialPort1);       // send packet to microcontroller

                SN_PacketIn = readPort(serialPort1);                // read data in from serial port

                // check for correct return header and valid checksum
                if(SN_PacketIn.command == 0x42 & SN_PacketIn.checksum_valid == true)
                {

                    status_PIC_SN.Text = SN_PacketIn.data[0].ToString();
                    // create a string with the received serial number
                    //string messageBoxText = "Serial Number: " + SN_PacketIn.data[0].ToString();
                    //string caption = "";    // create a blank string
                    // open a message box with the OK button and the string with the serial number
                    //MessageBox.Show(messageBoxText, caption, MessageBoxButtons.OK);
                }
            }
            else
            {
                // display message on text console to connect to PIC first
                textConsole.AppendText("e> Connect to PIC first." + Environment.NewLine);
            }
        }   // end of getSerialNumberToolStripMenuItem_Click

        /* Function: private void getFirmwareVersionToolStripMenuItem_Click(object sender, EventArgs e)
         *
         * Return Value:
         * 1. None
         *
         * Description: This function gets the firmware version from the microcontroller and displays it in a 
         * separate message box.*/
        private void getFirmwareVersion(object sender, EventArgs e)
        {
            if (Connected == true)
            {
                serialPort1.DiscardInBuffer();                      // clear serial port buffer
                Packet FW_PacketIn = new Packet();                  // create input packet structure

                Packet FW_Packet = new Packet(0x41);                // create packet
                FW_Packet.sendPacket(FW_Packet, serialPort1);       // send packet to microcontroller

                FW_PacketIn = readPort(serialPort1);                // read data in from serial port

                // check for correct return header and valid checksum
                if (FW_PacketIn.command == 0x41 & FW_PacketIn.checksum_valid == true)
                {
                    status_PIC_FW.Text = FW_PacketIn.data[0].ToString() + "." + FW_PacketIn.data[1].ToString("D2");
                    // create a string with the received firmware version
                    //string messageBoxText = "Firmware Version: " + FW_PacketIn.data[0] .ToString() + "." + FW_PacketIn.data[1].ToString();
                    //string caption = "";                            // create empty string
                    // open a message box with the OK button and the string with the firmware version
                    //MessageBox.Show(messageBoxText, caption, MessageBoxButtons.OK);
                }
            }
            else
            {
                // display message on text console to connect to PIC first
                textConsole.AppendText("e> Connect to PIC first." + Environment.NewLine);
            }
        }   // end of getFirmwareVersionToolStripMenuItem_Click


        //private void trackbar_Change(object sender, EventArgs e)
        //{
        //    voltage_Number.Value = (decimal)(voltage_TrackBar.Value / 1000.0);
        //    amplitude = (byte)(((double)voltage_Number.Value - 9.8) / 0.205);
        //    setVoltage(amplitude);

        //}   // end of trackbar_Change


        private void numeric_Change(object sender, EventArgs e)
        {
            amplitude = (byte)(voltage_Number.Value);
            if (amplitude == 0)
            {
                voltage_TB.Text = "0.000";
            }
            else
            {
                voltage_TB.Text = (((byte)voltage_Number.Value * 0.205) + 9.8).ToString("N3");
            }
            setVoltage(amplitude);

        }   // end of numeric_Change


        private void setVoltage(byte amplitude)
        {
            if (Connected == true)
            {
                serialPort1.DiscardInBuffer();                      // clear serial port buffer
                Packet Volt_PacketIn = new Packet();                  // create input packet structure
                byte[] data = new byte[1] { amplitude };
                Packet Volt_Packet = new Packet(0x50, 1, data);                // create packet

                Volt_Packet.sendPacket(Volt_Packet, serialPort1);       // send packet to microcontroller

                Volt_PacketIn = readPort(serialPort1);                // read data in from serial port

                // check for correct return header and valid checksum
                if (Volt_PacketIn.command == 0x50 & Volt_PacketIn.checksum_valid == true)
                {

                    switch (Volt_PacketIn.data[0])
                    {
                        case 0:
                            //textConsole.AppendText("m> Voltage Set." + Environment.NewLine);
                            break;
                        case 1:
                            textConsole.AppendText("e> Error setting voltage. Error Code: " + Volt_PacketIn.data[0].ToString() + Environment.NewLine);
                            break;

                        default:
                            break;
                    }
                    textConsole.ScrollToCaret();    // move console line to the end
                }
            }
            else
            {
                // display message on text console to connect to PIC first
                textConsole.AppendText("e> Connect to PIC first." + Environment.NewLine);
            }
        }   // end of setVoltage


        /* Function: public Packet readPort(SerialPort port)
         *
         * Input Arguments:
         * 1. port: This input is of the class SerialPort, and is intended to be the active serial port that the
         * program is connected to.
         * 
         * Return Value:
         * 1. tmpPacket: The return value is a variable of class Packet filled with the data read in from the 
         * serial port buffer.
         *
         * Description: This function reads the data in the serial port buffer and creates a Packet structure from 
         * that data.*/
        public Packet readPort(SerialPort port)
        {
            Packet tmpPacket;                               // create packet to pass data back to calling function
            int timeOutCounter = 0;                         // create timeout counter to exit function if there is 
                                                            // no data in the serial port buffer

            while (port.BytesToRead < 1)                    // check for bytes in serial port
            {
                timeOutCounter++;                           // increment counter
                System.Threading.Thread.Sleep(10);          // sleep for 10ms
                Application.DoEvents();                     // update the form and processes all messages currently in the message queue

                if (timeOutCounter > 100)                   // wait for a total of 1000ms before exiting the function
                {
                    textConsole.AppendText("e> Error: No response received." + Environment.NewLine);
                    tmpPacket = new Packet();               // create empty packet
                    return tmpPacket;                       // return empty packet
                }
            }   // end while

            System.Threading.Thread.Sleep(20);
            byte[] readdata = new byte[port.BytesToRead];   // create array based on the size of the data in the serial buffer

            port.Read(readdata, 0, port.BytesToRead);       // read the data from the port into the readdata array

            tmpPacket = new Packet(readdata, true);         // create packet with readdata data
            if(tmpPacket.checksum_valid == false)           // check for valid checksum
            {
                textConsole.AppendText("e> Error: Invalid Checksum." + Environment.NewLine);
            }
            return tmpPacket;                               // return packet

        } // end of readPort

    }   // end of partial class Form1:Form

}   // end of namespace
