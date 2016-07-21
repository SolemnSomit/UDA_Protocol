import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;

public class Server {

  public final static int SOCKET_PORT = 13268;  // you may change this
  // public final static int SOCKET_PORT = 8080;  // you may change this
  public final static String FILE_TO_SEND = "three.txt";  // you may change this

  public static void main (String [] args ) throws IOException {
    FileInputStream fis = null;
    BufferedInputStream bis = null;
    OutputStream os = null;
    ServerSocket servsock = null;
    Socket sock = null;
    try {
      servsock = new ServerSocket(SOCKET_PORT);
      while (true) {
        System.out.println("Waiting...");
        try {
          sock = servsock.accept();
          System.out.println("Accepted connection : " + sock);
          // send file
          File myFile = new File (FILE_TO_SEND);
          byte [] mybytearray  = new byte [(int)myFile.length()];
          fis = new FileInputStream(myFile);
          bis = new BufferedInputStream(fis);
          bis.read(mybytearray,0,mybytearray.length);
          os = sock.getOutputStream();
          System.out.println("Sending " + FILE_TO_SEND + "(" + mybytearray.length + " bytes)");
          long startTime = System.currentTimeMillis();
          os.write(mybytearray,0,mybytearray.length);
          os.flush();
          long stopTime = System.currentTimeMillis();
          long elapsedTime = stopTime - startTime;
          System.out.println(elapsedTime);
          float fileSize = Float.valueOf(mybytearray.length);
          System.out.println("Avg speed = " + fileSize/(elapsedTime*1000) + "MBPS") ;
          System.out.println("Done.");
        }
        finally {
          if (bis != null) bis.close();
          if (os != null) os.close();
          if (sock!=null) sock.close();
        }
      }
    }
    finally {
      if (servsock != null) servsock.close();
    }
  }
}