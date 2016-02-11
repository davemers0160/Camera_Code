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
using System.Timers;


namespace Packet_Class
{
    // Packet Class:  This class is designed to handle all of the packet operations and overloaded functions
    public class Packet
    {
        public byte start;              // start byte
        public byte command;            // command byte;
        public byte byte_count;         // byte_count byte;
        public byte[] data;             // data array
        public ushort checksum;         // checksum;
        public bool checksum_valid;     // valid checksum true/false

        /* Function: public Packet()
         *
         * Input Arguments:
         * 1. None
         * 
         * Return Value:
         * 1. Packet: The return value is a variable of class Packet.
         *
         * Description: This function is one of the overloaded methods for the creation of a Packet variable.  This
         * version takes no inputs and creates an empty structure. */  
        public Packet()
        {
            start = 0;
            command = 0;
            byte_count = 0;
            data = null;
        }

        /* Function: public Packet(byte cmd)
         *
         * Input Arguments:
         * 1. cmd: This variable is used to populate the command variable
         * 
         * Return Value:
         * 1. Packet: The return value is a variable of class Packet.
         *
         * Description: This function is one of the overloaded methods for the creation of a Packet variable.  This
         * version takes in the command variable.  This method is intended to be used when no data will be sent. */  
        public Packet(byte cmd)
        {
            start = (byte)'$';
            command = cmd;
            byte_count = 0;
            data = null;
        }

        /* Function: public Packet(byte cmd, byte bc, byte[] dt)
         *
         * Input Arguments:
         * 1. cmd: This variable is used to populate the command variable
         * 2. bc: This variable is used to populate the byte_count variable
         * 3. dt: This variable is used to populate the data array variable 
         * 
         * Return Value:
         * 1. Packet: The return value is a variable of class Packet.
         *
         * Description: This function is one of the overloaded methods for the creation of a Packet variable.  This
         * version takes in the command, byte_count and data variables.  This method is intended to be used when all the
         * information is available to create a packet. */  
        public Packet(byte cmd, byte bc, byte[] dt)
        {
            start = (byte)'$';
            command = cmd;
            byte_count = bc;
            data = dt;
        }

        /* Function: public Packet(byte[] data_in, bool check)
         *
         * Input Arguments:
         * 1. data_in: This byte array variable is used to create a packet and contains all of the data, including 
         *             the command, byte_count and data[] variables.
         * 2. check: This boolean variable is used to generate a checksum based on the input data
         * 
         * Return Value:
         * 1. Packet: The return value is a variable of class Packet.
         *
         * Description: This function is one of the overloaded methods for the creation of a Packet variable.  This
         * version takes byte array variable.  This method is intended to be used when data is read in from the serial
         * port in a byte array format.  The data may or may not contain a checksum. */  
        public Packet(byte[] data_in, bool check)
        {
            command = data_in[0];           // fill in command
            byte_count = data_in[1];        // fill in byte_count
            if (check == true)              // check check.  If true generate checksum and check against passed in checksum
            {
                byte_count -= 2;            // account for checksum in the size of the data array
                data = new byte[byte_count];                    // create data array
                Array.Copy(data_in, 2, data, 0, byte_count);    // copy data from data_in array to data array
                checksum_valid = checkFletcher(data_in);        // call checkFletcher function
                checksum = (ushort)(data_in[data_in.Length - 2] << 8);  // fill in checksum
                checksum += (ushort)(data_in[data_in.Length - 1]);
            }
            else
            {
                data = new byte[byte_count];                    // create data array
                Array.Copy(data_in, 2, data, 0, byte_count);    // copy data from data_in array to data array
                checksum_valid = true;                          // set checksum valid to true
                checksum = 0;                                   // fill in dummy checksum value
            }
        }   // end of Packet

        /* Function: public void sendPacket(Packet send, SerialPort Port)
         *
         * Input Arguments:
         * 1. send: This Packet variable is what will be sent out on the specified serial port 
         * 2. Port: This SerialPort variable the serial port that data will be sent out on.
         * 
         * Return Value:
         * 1. None
         *
         * Description: This function is designed to send a data packet on the specified serial port. No checksum
         * is intended to be sent out. */  
        public void sendPacket(Packet send, SerialPort Port)
        {                                
            if(send.checksum == 0)
            {
                byte[] packet_out = new byte[send.byte_count + 3];  // create array to send out of serial port
                packet_out[0] = send.start;                         // populate array in order
                packet_out[1] = send.command;
                packet_out[2] = send.byte_count;
                
                if (send.data != null)
                {
                    send.data.CopyTo(packet_out, 3);                // copy data variable into packet_out
                }

                Port.Write(packet_out, 0, send.byte_count + 3);
                /*
                for (int i = 0; i < send.byte_count + 3; i++)       // add 3 bytes for start, command and byte_count
                {
                    Port.Write(packet_out, i, 1);                   // write individual byte to port
                    System.Threading.Thread.Sleep(10);              // sleep 10ms before sending another byte.  This makes
                                                                    // sure that micorcontroller has enough time to receive data
                }  
                */
            }
        }   // end of sendPacket

        /* Function: private bool checkFletcher(byte[] packetBytes)
         *
         * Input Arguments:
         * 1. packetBytes: This byte array variable is the data to perform the Fletcher checksum calculations on 
         * 
         * Return Value:
         * 1. validchecksum: This boolean variable indicates if the checksum calculated matches the checksum in 
         *                   the packetBytes array.
         *
         * Description: This function is designed to calculated the 16-bit Fletcher checksum based on the data passed 
         * into the function. The function returns true if the passed in checksum equals the calculated checksum,
         * otherwise false. */
        private bool checkFletcher(byte[] packetBytes)
        {
            ushort sum1 = 0;
            ushort sum2 = 0;
            ushort calculatedChecksum, fletcherChecksum;

            // run calculation for all bytes except the last two which should be the checksum passed
            // in by the microcontroller
            for (int index = 0; index < packetBytes.Length - 2; ++index)
            {
                sum1 = (ushort)((sum1 + packetBytes[index]) % (byte)(255));
                sum2 = (ushort)((sum2 + sum1) % (byte)(255));
            }

            // generate final checksum
            calculatedChecksum = (ushort)((sum2 << 8) | sum1);

            // generate passed in checksum
            fletcherChecksum = (ushort)(packetBytes[packetBytes.Length - 2] << 8);
            fletcherChecksum = (ushort)(fletcherChecksum + packetBytes[packetBytes.Length - 1]);

            if (fletcherChecksum == calculatedChecksum)     // compare the two values
            {
                return true;
            }
            else
            {
                return false;
            }
        }   // end of checkFletcher
    }   // end of Packet class
}   // end of namespace
