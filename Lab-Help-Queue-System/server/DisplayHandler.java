package server;


import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.text.SimpleDateFormat;
import java.util.Comparator;
import java.util.Date;

/**
 * @author qiz9744, wangs4830
 */
class DisplayHandler implements Runnable {
    final DataInputStream dis;
    final DataOutputStream dos;
    Socket s;

    /**
     * constructor
     */
    public DisplayHandler(Socket s, DataInputStream dis, DataOutputStream dos) {
        this.s = s;
        this.dis = dis;
        this.dos = dos;
    }

    @Override
    public void run() {
        while (true) {

            try {
                String received = dis.readUTF();
                String sent = "#";
                if (received.equals("disconnect")) {
                    break;
                } else if (received.equals("update")) {
                    Server.clientVector.sort(new Comparator<ClientData>() {
                        @Override
                        public int compare(ClientData o1, ClientData o2) {
                            if (o1.getDate() == null || o2.getDate() == null) {
                                return 0;
                            }
                            return o1.getDate().compareTo(o2.getDate());
                        }
                    });
                    for (int i = 0; i < Server.clientVector.size(); i++) {
                        String name = Server.clientVector.get(i).getName();
                        Date date = Server.clientVector.get(i).getDate();
                        if (date != null) {
                            String formatTime = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(date);
                            sent = sent.concat(name + "," + formatTime + "#");
                        }
                    }
                    dos.writeUTF("Subject: " + Server.currentClass);
                    dos.writeUTF("Section: " + Server.currentSection);
                    dos.writeUTF(sent);
                } else {
                    dos.writeUTF("Message: unknown command - " + received);
                }
            } catch (IOException e) {
                break;
            } catch (Exception ignored) {
            }
        }

        try {
            dis.close();
            dos.close();
        } catch (IOException ignored) {
        }
    }

}
