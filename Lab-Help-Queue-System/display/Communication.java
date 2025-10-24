package display;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.concurrent.TimeUnit;

/**
 * @author qiz9744, wangs4830
 */
public class Communication {
    static DataOutputStream dos;
    static DataInputStream dis;
    final int SERVER_PORT = 3030;

    /**
     * constructor
     */
    public Communication(String ip) {
        Socket s;
        try {
            s = new Socket(InetAddress.getByName(ip), SERVER_PORT);
            dos = new DataOutputStream(s.getOutputStream());
            dis = new DataInputStream(s.getInputStream());
        } catch (IOException e) {
            Gui.setMessage("cannot find server");
            try {
                TimeUnit.SECONDS.sleep(3);
            } catch (InterruptedException ignored) {
            }
            System.exit(0);
        }

        // send thread
        Thread send = new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    try {
                        dos.writeUTF("update");
                        TimeUnit.SECONDS.sleep(3);
                    } catch (IOException | InterruptedException e) {
                        Gui.setMessage("cannot find server");
                        try {
                            TimeUnit.SECONDS.sleep(3);
                        } catch (InterruptedException ignored) {
                        }
                        System.exit(0);
                    }
                }
            }
        });

        // receive thread
        Thread receive = new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    try {
                        String msg = dis.readUTF();
                        Gui.clearLabels();
                        if (msg.startsWith("Subject:")) {
                            Gui.setSubject(msg);
                        } else if (msg.startsWith("Section:")) {
                            Gui.setSection(msg);
                        } else if (msg.startsWith("Message:")) {
                            Gui.setMessage(msg);
                        } else if (msg.startsWith("#") && msg.length() > 1) {
                            msg = msg.substring(1, msg.length() - 1);
                            String[] column = msg.split("#");
                            for (int i = 0; i < column.length; i++) {
                                String[] row = column[i].split(",");
                                Date time = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").parse(row[1]);
                                long minus = (System.currentTimeMillis() - time.getTime());
                                long minute = minus / 1000 / 60;
                                long second = minus / 1000 % 60;
                                String formatRow = "   " + String.format("%02d", i + 1) +
                                        "      " + row[0] +
                                        "    " + new SimpleDateFormat("HH:mm:ss").format(time) +
                                        "       " + String.format("%03d", minute) + ":" + String.format("%02d", second);
                                if (i < 10) {
                                    Gui.setLabels(i, formatRow);
                                }
                            }
                        }
                    } catch (IOException | ParseException exception) {
                        Gui.setMessage("cannot find server");
                        try {
                            TimeUnit.SECONDS.sleep(3);
                        } catch (InterruptedException ignored) {
                        }
                        System.exit(0);
                    }
                }
            }
        });

        send.start();
        receive.start();
    }
}

