/*******************************************************************************
 Copyright 2019 Philipp Kutsch

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 /********************************************************************************/

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.Path;
import java.io.IOException;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferByte;
import java.io.IOException;
import javax.imageio.ImageIO;
import java.io.File;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.lang.NumberFormatException;

/**
 * Crude program to convert a simple bmp image into an c-array of vga 256 colors.
 *
 * Usage:
 * BmpToArray <source bmp file> [image width] [image height] [array name]
 */
public class BmpToArray
{
    public static void main(String[] args)
    {
        //Parse and read arguments
        if(args.length != 4)
        {
            printUsage();
            return;
        }

        String filename = args[0];

        int x = 0;
        int y = 0;
        try
        {
            x = Integer.parseInt(args[1]);
            y = Integer.parseInt(args[2]);

            if(x <= 0 || y <= 0)
                throw new NumberFormatException("Image size have to be a positive number");
        }
        catch(NumberFormatException e)
        {
            e.printStackTrace();
            printUsage();
            return;
        }

        //Read file content into buffer
        byte[] data;
        try
        {
            Path path = Paths.get(filename);
            data = Files.readAllBytes(path);
        }
        catch(IOException e)
        {
            e.printStackTrace();
            printUsage();
            return;
        }

        //Try to parse bmp file
        BmpFile bmpFile;
        try
        {
            bmpFile = BmpFile.fromBytes(data);
        }
        catch (RuntimeException e)
        {
            e.printStackTrace();
            return;
        }

        //Only uncompressed bitmaps are supported at the moment
        if(bmpFile.getInformationHeader().getBiCompression() != BmpFile.InformationHeader.BI_RGB)
        {
            System.out.println("Only uncompressed bitmaps are supported at the moment");
            return;
        }

        //Only 8 Bit color mode (256 colors) are supported by vga mode 13h
        if(bmpFile.getInformationHeader().getBiBitCount() != 8)
        {
            System.out.println("Only 8Bit color palette mode is supported.");
            return;
        }

        System.out.println("--- Header ---");
        System.out.println("bfType: " + bmpFile.getHeader().getBfType());
        System.out.println("bfSize: " + bmpFile.getHeader().getBfSize());
        System.out.println("bfReserved: " + bmpFile.getHeader().getBfReserved());
        System.out.println("bfOffBits: " + bmpFile.getHeader().getBfOffBits());

        System.out.println("--- Information Header ---");
        System.out.println("biSize: " + bmpFile.getInformationHeader().getBiSize());
        System.out.println("biWidth: " + bmpFile.getInformationHeader().getBiWidth());
        System.out.println("biHeight: " + bmpFile.getInformationHeader().getBiHeight());
        System.out.println("biPlanes: " + bmpFile.getInformationHeader().getBiPlanes());
        System.out.println("biBitCount: " + bmpFile.getInformationHeader().getBiBitCount());
        System.out.println("biCompression: " + bmpFile.getInformationHeader().getBiCompression());
        System.out.println("biSizeImage: " + bmpFile.getInformationHeader().getBiSizeImage());
        System.out.println("biXPelsPerMeter: " + bmpFile.getInformationHeader().getBiXPelsPerMeter());
        System.out.println("biYPelsPerMeter: " + bmpFile.getInformationHeader().getBiYPelsPerMeter());
        System.out.println("biClrUsed: " + bmpFile.getInformationHeader().getBiClrUsed());
        System.out.println("biClrImportant: " + bmpFile.getInformationHeader().getBiClrImportant());

        //@TODO

        File logFile = new File("output");
        try(BufferedWriter writer = new BufferedWriter(new FileWriter(logFile)))
        {
            //@TODO read bmp header and read image size from header
            int default_offset = 1024 + 54 + 68;
            int image_width = x;
            int image_height = y;
            int padding_bytes = image_width % 4;

            String arrayBegin = "uint8_t " + args[3] + " [" + (image_width * image_height) + "] = {";
            writer.write(arrayBegin);
            System.out.print(arrayBegin);

            int bytes_per_line = 0;
            int wrote_bytes = 0;
            for(int i = default_offset; i < data.length; i++)
            {
                boolean ignore_byte = false;

                if(padding_bytes == 1)
                {
                    if(bytes_per_line == image_width)
                    {
                        ignore_byte = true;
                        bytes_per_line = 0;
                    }
                    else
                    {
                        bytes_per_line++;
                    }
                }
                else if(padding_bytes == 2)
                {
                    if(bytes_per_line == image_width)
                    {
                        ignore_byte = true;
                        bytes_per_line++;
                    }
                    else if(bytes_per_line == image_width + 1)
                    {
                        ignore_byte = true;
                        bytes_per_line = 0;
                    }
                    else
                    {
                        bytes_per_line++;
                    }
                }
                else if(padding_bytes == 3)
                {
                    if(bytes_per_line == image_width)
                    {
                        ignore_byte = true;
                        bytes_per_line++;
                    }
                    else if(bytes_per_line == image_width + 1)
                    {
                        ignore_byte = true;
                        bytes_per_line++;
                    }
                    else if(bytes_per_line == image_width + 2)
                    {
                        ignore_byte = true;
                        bytes_per_line = 0;
                    }
                    else
                    {
                        bytes_per_line++;
                    }
                }

                if(!ignore_byte)
                {
                    if(i < data.length -1)
                    {
                        String out = "0x" + Integer.toHexString(data[i] & 0xff) + ", ";
                        writer.write(out);
                        System.out.print(out);
                    }
                    else
                    {
                        String out = "0x" + Integer.toHexString(data[i] & 0xff);
                        writer.write(out);
                        System.out.print(out);
                    }

                    wrote_bytes++;
                }
            }

            writer.write("};");
            System.out.println("};");

        }
        catch (Exception e)
        {
            e.printStackTrace();
            return;
        }
    }

    private static void printUsage()
    {
        System.out.println("Usage:");
        System.out.println("BmpToArray <source bmp file> [image width] [image height] [array name]");
    }

    @SuppressWarnings("unused")
    private static class BmpFile
    {
        private static final int BMP_VERSION = 3;

        private static final int HEADER_SIZE = 14;
        private static final int INFORMATION_HEADER_SIZE = 40;

        private Header header;
        private InformationHeader informationHeader;
        private byte[] imageData;

        static BmpFile fromBytes(byte[] bmpBytes)
        {
            BmpFile bmpFile = new BmpFile();

            byte[] headerBytes = new byte[HEADER_SIZE];
            System.arraycopy(bmpBytes, 0, headerBytes, 0, HEADER_SIZE);

            byte[] informationHeaderBytes = new byte[INFORMATION_HEADER_SIZE];
            System.arraycopy(bmpBytes, HEADER_SIZE, informationHeaderBytes, 0, INFORMATION_HEADER_SIZE);

            bmpFile.setHeader(Header.fromBytes(headerBytes));
            bmpFile.setInformationHeader(InformationHeader.fromBytes(informationHeaderBytes));

            //If image size is stored in the information header structure
            int imageDataSize;
            if(bmpFile.getInformationHeader().getBiSizeImage() != 0)
            {
                imageDataSize = bmpFile.getInformationHeader().getBiSizeImage();
            }
            else
            {
                imageDataSize = bmpBytes.length - bmpFile.getHeader().getBfOffBits();
            }

            byte[] imageData = new byte[imageDataSize];
            System.arraycopy(bmpBytes, bmpFile.getHeader().getBfOffBits(), imageData, 0, imageDataSize);

            bmpFile.setImageData(imageData);

            return bmpFile;
        }

        Header getHeader()
        {
            return header;
        }

        void setHeader(Header header)
        {
            this.header = header;
        }

        InformationHeader getInformationHeader()
        {
            return informationHeader;
        }

        void setInformationHeader(InformationHeader informationHeader)
        {
            this.informationHeader = informationHeader;
        }

        byte[] getImageData()
        {
            return imageData;
        }

        void setImageData(byte[] imageData)
        {
            this.imageData = imageData;
        }

        private static class Header
        {
            private static final String HEADER_MAGIC_NUMBER = "BM";

            private String bfType;
            private int bfSize;
            private int bfReserved;
            private int bfOffBits;

            static Header fromBytes(byte[] headerBytes)
            {
                if(headerBytes.length != HEADER_SIZE)
                    throw new IllegalArgumentException("Header invalid size.");

                Header header = new Header();

                byte[] typeBytes = new byte[2];
                System.arraycopy(headerBytes, 0, typeBytes, 0, 2);
                header.setBfType(new String(typeBytes, StandardCharsets.US_ASCII));

                if(!header.getBfType().equals(HEADER_MAGIC_NUMBER))
                    throw new IllegalArgumentException("Header invalid magic number");

                header.setBfSize(getIntegerAtOffsetFromByteArray(headerBytes, 2));
                header.setBfReserved(getIntegerAtOffsetFromByteArray(headerBytes, 6));
                header.setBfOffBits(getIntegerAtOffsetFromByteArray(headerBytes, 10));



                return header;
            }

            String getBfType()
            {
                return bfType;
            }

            void setBfType(String bfType)
            {
                this.bfType = bfType;
            }

            int getBfSize()
            {
                return bfSize;
            }

            void setBfSize(int bfSize)
            {
                this.bfSize = bfSize;
            }

            int getBfReserved()
            {
                return bfReserved;
            }

            void setBfReserved(int bfReserved)
            {
                this.bfReserved = bfReserved;
            }

            int getBfOffBits()
            {
                return bfOffBits;
            }

            void setBfOffBits(int bfOffBits)
            {
                this.bfOffBits = bfOffBits;
            }
        }

        private static class InformationHeader
        {
            //Compression types
            //Uncompressed
            static final int BI_RGB = 0;

            //run-length encoding
            static final int BI_RLE8 = 1;

            //run-length encoding
            static final int BI_RLE4 = 2;

            //uncompressed with color mask
            static final int BI_BITFIELDS = 3;

            //size of the information header structure in bytes
            private int biSize;

            //|biWidth| image width
            private int biWidth;

            //|biHeight| image height
            //If biHeight is positive the bitmap is bottom-up.
            //Else top-down
            private int biHeight;

            //unused
            private short biPlanes;

            //Color depth 1, 4, 8, 16, 24 or 32
            private short biBitCount;

            //compression type
            private int biCompression;

            //image size in bytes
            private int biSizeImage;

            //default 0
            private int biXPelsPerMeter;

            //default 0
            private int biYPelsPerMeter;

            //color table entries
            private int biClrUsed;

            //colors used in the image
            private int biClrImportant;

            static InformationHeader fromBytes(byte[] informationBytes)
            {
                if(informationBytes.length != INFORMATION_HEADER_SIZE)
                    throw new IllegalArgumentException("Information Header invalid size.");

                InformationHeader informationHeader = new InformationHeader();
                informationHeader.setBiSize(getIntegerAtOffsetFromByteArray(informationBytes, 0));
                informationHeader.setBiWidth(getIntegerAtOffsetFromByteArray(informationBytes, 4));
                informationHeader.setBiHeight(getIntegerAtOffsetFromByteArray(informationBytes, 8));
                informationHeader.setBiPlanes(getShortAtOffsetFromByteArray(informationBytes, 12));
                informationHeader.setBiBitCount(getShortAtOffsetFromByteArray(informationBytes, 14));
                informationHeader.setBiCompression(getIntegerAtOffsetFromByteArray(informationBytes, 16));
                informationHeader.setBiSizeImage(getIntegerAtOffsetFromByteArray(informationBytes, 20));
                informationHeader.setBiXPelsPerMeter(getIntegerAtOffsetFromByteArray(informationBytes, 24));
                informationHeader.setBiYPelsPerMeter(getIntegerAtOffsetFromByteArray(informationBytes, 28));
                informationHeader.setBiClrUsed(getIntegerAtOffsetFromByteArray(informationBytes, 32));
                informationHeader.setBiClrImportant(getIntegerAtOffsetFromByteArray(informationBytes, 36));

                return informationHeader;
            }

            int getBiSize()
            {
                return biSize;
            }

            void setBiSize(int biSize)
            {
                this.biSize = biSize;
            }

            int getBiWidth()
            {
                return biWidth;
            }

            void setBiWidth(int biWidth)
            {
                this.biWidth = biWidth;
            }

            int getBiHeight()
            {
                return biHeight;
            }

            void setBiHeight(int biHeight)
            {
                this.biHeight = biHeight;
            }

            short getBiPlanes()
            {
                return biPlanes;
            }

            void setBiPlanes(short biPlanes)
            {
                this.biPlanes = biPlanes;
            }

            short getBiBitCount()
            {
                return biBitCount;
            }

            void setBiBitCount(short biBitCount)
            {
                this.biBitCount = biBitCount;
            }

            int getBiCompression()
            {
                return biCompression;
            }

            void setBiCompression(int biCompression)
            {
                this.biCompression = biCompression;
            }

            int getBiSizeImage()
            {
                return biSizeImage;
            }

            void setBiSizeImage(int biSizeImage)
            {
                this.biSizeImage = biSizeImage;
            }

            int getBiXPelsPerMeter()
            {
                return biXPelsPerMeter;
            }

            void setBiXPelsPerMeter(int biXPelsPerMeter)
            {
                this.biXPelsPerMeter = biXPelsPerMeter;
            }

            int getBiYPelsPerMeter()
            {
                return biYPelsPerMeter;
            }

            void setBiYPelsPerMeter(int biYPelsPerMeter)
            {
                this.biYPelsPerMeter = biYPelsPerMeter;
            }

            int getBiClrUsed()
            {
                return biClrUsed;
            }

            void setBiClrUsed(int biClrUsed)
            {
                this.biClrUsed = biClrUsed;
            }

            int getBiClrImportant()
            {
                return biClrImportant;
            }

            void setBiClrImportant(int biClrImportant)
            {
                this.biClrImportant = biClrImportant;
            }
        }

        private static int getIntegerAtOffsetFromByteArray(byte[] bytes, int offset)
        {
            byte[] integerBytes = new byte[4];
            System.arraycopy(bytes, offset, integerBytes, 0, 4);
            ByteBuffer integerByteBuffer = ByteBuffer.wrap(integerBytes);
            integerByteBuffer.order(ByteOrder.LITTLE_ENDIAN);
            return integerByteBuffer.getInt();
        }

        private static short getShortAtOffsetFromByteArray(byte[] bytes, int offset)
        {
            byte[] shortBytes = new byte[2];
            System.arraycopy(bytes, offset, shortBytes, 0, 2);
            ByteBuffer shortByteBuffer = ByteBuffer.wrap(shortBytes);
            shortByteBuffer.order(ByteOrder.LITTLE_ENDIAN);
            return shortByteBuffer.getShort();
        }
    }
}
