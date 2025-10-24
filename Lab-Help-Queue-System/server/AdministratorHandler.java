package server;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.sql.SQLException;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * @author qiz9744, wangs4830
 */
class AdministratorHandler implements Runnable {
    final DataInputStream dis;
    final DataOutputStream dos;
    Socket s;
    boolean logged = false;

    /**
     * constructor
     */
    public AdministratorHandler(Socket s, DataInputStream dis, DataOutputStream dos) {
        this.s = s;
        this.dis = dis;
        this.dos = dos;
    }

    @Override
    public void run() {
        while (true) {
            try {
                String received = dis.readUTF();
                if (received.equals("disconnect")) {
                    break;
                } else if (received.equals("update")) {
                    dos.writeUTF("Subject: " + Server.currentClass);
                    dos.writeUTF("Section: " + Server.currentSection);
                } else if (!logged && received.startsWith("login: ")) {
                    logIn(received);
                } else if (received.startsWith("login: ")) {
                    dos.writeUTF("Message: already logged in");
                } else if (!logged) {
                    dos.writeUTF("Message: login first");
                } else if (received.startsWith("cancel: ")) {
                    cancelHelp(received);
                } else if (received.startsWith("add: ")) {
                    addSubject(received);
                } else if (received.equals("initialize queue")) {
                    Server.initialize();
                    dos.writeUTF("Message: initialize help queue success");
                } else if (received.equals("initialize subject")) {
                    Server.t.initializeSubject();
                    dos.writeUTF("Message: initialize subject success");
                } else if (received.equals("initialize event")) {
                    Server.t.initializeEvent();
                    dos.writeUTF("Message: initialize event success");
                } else {
                    dos.writeUTF("Message: unknown command - " + received);
                }
            } catch (IOException e) {
                break;
            } catch (Exception e) {
                try {
                    dos.writeUTF("Message: action fail");
                } catch (IOException ignored) {

                }
            }

        }
        try {
            dis.close();
            dos.close();
        } catch (IOException ignored) {
        }
    }

    /**
     * log in
     *
     * @param received received
     * @throws SQLException
     * @throws IOException
     */
    private void logIn(String received) throws SQLException, IOException {
        received = received.substring(7);
        String[] strs = received.split("\n");
        String correctPassword = Server.t.login(strs[0]);
        correctPassword = correctPassword.substring(0, correctPassword.length() - 3);
        if (strs[1].equals(correctPassword)) {
            logged = true;
            dos.writeUTF("Message: login success");
        } else {
            dos.writeUTF("Message: login fail");
        }
    }

    /**
     * add subject
     *
     * @param received content
     * @throws IOException
     */
    private void addSubject(String received) throws IOException, SQLException {
        // add:Sunday#13:00:00,15:00:00#2020-01-01,2020-12-31#CS 260#1
        // remove "add: "
        received = received.substring(5);
        String[] str = received.split("#");
        // CS 260
        // 1
        // 13:00:00,15:00:00
        // Sunday
        // 2020-01-01,2020-12-31
        String subject = str[0];
        int section = Integer.parseInt(str[1]);
        String startTime = str[2].split(",")[0];
        String endTime = str[2].split(",")[1];
        String week = str[3];
        String startDate = str[4].split(",")[0];
        String endDate = str[4].split(",")[1];
        Server.t.insertSubject(subject, section, startTime, endTime, startDate, endDate, week);
        dos.writeUTF("Message: add subject success");
    }

    /**
     * cancel help
     *
     * @param received content
     * @throws IOException
     */
    private void cancelHelp(String received) throws IOException, SQLException {
        // delete "cancel: "
        received = received.substring(8);
        // look for client from clientVector
        int index = Server.clientVector.indexOf(new ClientData(received));
        // if cannot find it
        if (index == -1) {
            dos.writeUTF("Message: cannot find workstation");
        }
        // if find it
        else {
            ClientData clientData = Server.clientVector.get(index);
            if (clientData.isHelping()) {
                Server.t.insertEvent("CANCEL", clientData.getName(),
                        "ADMIN",
                        Server.currentClass,
                        Server.currentSection,
                        new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()),
                        Server.calculateWaitTime(clientData));
                // change status in clientVector
                clientData.setHelping(false);
                clientData.setDate(null);
                dos.writeUTF("Message: cancel help successfully");
            } else {
                dos.writeUTF("Message: cannot cancel non-existent help request");
            }
        }
    }
}


