package client;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.util.concurrent.TimeUnit;

/**
 * @author qiz9744, wangs4830
 */
public class Communication {
    static DataOutputStream dos;
    static DataInputStream dis;
    final int SERVER_PORT = 3020;

    /**
     * constructor
     */
    public Communication(String ip) {
        Socket s;
        try {
            s = new Socket(InetAddress.getByName(ip), SERVER_PORT);
            dos = new DataOutputStream(s.getOutputStream());
            dis = new DataInputStream(s.getInputStream());
            Gui.setButtonEnabled();
            send("update");
        } catch (IOException e) {
            connectFail();
        }

        // receive thread
        Thread receive = new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    try {
                        String str = dis.readUTF();
                        if (str.startsWith("Subject:")) {
                            Gui.setSubject(str);
                        } else if (str.startsWith("Section:")) {
                            Gui.setSection(str);
                        } else if (str.startsWith("Message:")) {
                            Gui.setMessage(str);
                        }
                    } catch (IOException e) {
                        connectFail();
                    }
                }
            }
        });

        // update thread
        Thread update = new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    try {
                        dos.writeUTF("update");
                        TimeUnit.SECONDS.sleep(3);
                    } catch (IOException | InterruptedException e) {
                        connectFail();
                    }
                }
            }
        });
        update.start();
        receive.start();
    }

    /**
     * send content to server
     *
     * @param content content
     */
    public static void send(String content) {
        Thread send = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    dos.writeUTF(content);
                } catch (IOException exception) {
                    connectFail();
                }
            }
        });
        send.start();
    }

    /**
     * connect server fail
     */
    public static void connectFail() {
        Gui.setMessage("Message: cannot find server");
        Gui.setButtonDisabled();
        try {
            TimeUnit.SECONDS.sleep(3);
        } catch (InterruptedException ignored) {
        }
        System.exit(0);
    }

}
