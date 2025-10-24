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
class ClientHandler implements Runnable {
    final DataInputStream dis;
    final DataOutputStream dos;
    Socket s;

    /**
     * constructor
     */
    public ClientHandler(Socket s, DataInputStream dis, DataOutputStream dos) {
        this.s = s;
        this.dis = dis;
        this.dos = dos;
    }

    @Override
    public void run() {
        int index = Server.clientVector.indexOf(new ClientData(this));
        ClientData clientData = Server.clientVector.get(index);

        while (true) {
            try {
                String received = dis.readUTF();
                if ("disconnect".equals(received)) {
                    break;
                } else if ("request".equals(received)) {
                    requestHelp(clientData);
                } else if ("cancel".equals(received)) {
                    cancelHelp(clientData);
                } else if ("update".equals(received)) {
                    dos.writeUTF("Subject: " + Server.currentClass);
                    dos.writeUTF("Section: " + Server.currentSection);
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
            if (clientData.isHelping()) {
                Server.t.insertEvent("CANCEL",
                        clientData.getName(),
                        "SYSTEM",
                        Server.currentClass,
                        Server.currentSection,
                        new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()),
                        Server.calculateWaitTime(clientData));
            }
            clientData.setHelping(false);
            clientData.setDate(null);
            clientData.setOnline(false);
            dis.close();
            dos.close();
        } catch (Exception ignored) {
        }

    }

    /**
     * cancel help
     *
     * @param clientData clientData
     * @throws IOException
     * @throws SQLException
     */
    private void cancelHelp(ClientData clientData) throws IOException, SQLException {
        if (clientData.isHelping()) {
            Server.t.insertEvent("CANCEL",
                    clientData.getName(),
                    "CLIENT",
                    Server.currentClass,
                    Server.currentSection,
                    new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()),
                    Server.calculateWaitTime(clientData));
            clientData.setHelping(false);
            clientData.setDate(null);
            dos.writeUTF("Message: cancel help successfully");
        } else {
            dos.writeUTF("Message: cannot cancel non-existent help request");
        }
    }

    /**
     * request help
     *
     * @param clientData clientData
     * @throws IOException
     * @throws SQLException
     */
    private void requestHelp(ClientData clientData) throws IOException, SQLException {
        if (clientData.isHelping()) {
            dos.writeUTF("Message: duplicate help request");
        } else {
            clientData.setHelping(true);
            clientData.setDate(new Date());
            Server.t.insertEvent("HELP",
                    clientData.getName(),
                    "CLIENT",
                    Server.currentClass,
                    Server.currentSection,
                    new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(clientData.getDate()),
                    null);
            dos.writeUTF("Message: request help successfully");
        }

    }
}
