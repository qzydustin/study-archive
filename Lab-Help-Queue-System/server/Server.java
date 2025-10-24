package server;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.sql.SQLException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Vector;
import java.util.concurrent.TimeUnit;

/**
 * @author qiz9744, wangs4830
 */
public class Server {
    final static int CLIENT_PORT = 3020;
    final static int DISPLAY_PORT = 3030;
    final static int ADMINISTRATOR_PORT = 3040;
    static Vector<ClientData> clientVector = new Vector<>();
    static PreparedStatement t;
    static String currentClass = "No Class";
    static int currentSection = 0;


    public static void main(String[] args) {
        DataAccessObject dao = new DataAccessObject();
        try {
            dao.connect(args[0], args[1]);
        } catch (Exception e) {
            System.out.println("cannot connect to the database");
            return;
        }

        dao.setAutoCommit(false);
        t = new PreparedStatement(dao);

        Thread clientThread = new Thread(new ClientRunnable());
        Thread displayThread = new Thread(new DisplayRunnable());
        Thread administratorThread = new Thread(new AdminRunnable());
        Thread updateSubjectAndSection = new Thread(new UpdateRunnable());

        clientThread.start();
        displayThread.start();
        administratorThread.start();
        updateSubjectAndSection.start();
        System.out.println("server started successfully");
    }

    /**
     * initialize server
     */
    public static void initialize() throws SQLException {
        for (int i = 0; i < Server.clientVector.size(); i++) {
            if (Server.clientVector.get(i).isHelping()) {
                Server.clientVector.get(i).setHelping(false);
                Server.t.insertEvent("CANCEL",
                        Server.clientVector.get(i).getName(),
                        "SYSTEM",
                        Server.currentClass,
                        Server.currentSection,
                        new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()),
                        calculateWaitTime(Server.clientVector.get(i)));
                Server.clientVector.get(i).setHelping(false);
                Server.clientVector.get(i).setDate(null);
            }

        }
    }

    /**
     * calculate client wait time
     *
     * @param clientData client data
     * @return oracle command
     */
    public static String calculateWaitTime(ClientData clientData) {
        long mss = (System.currentTimeMillis() - clientData.getDate().getTime());
        long days = mss / (1000 * 60 * 60 * 24);
        long hours = (mss % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60);
        long minutes = (mss % (1000 * 60 * 60)) / (1000 * 60);
        long seconds = mss % (1000 * 60) / 1000;
        return "INTERVAL '" + days + " " + hours + ":" + minutes + ":" + seconds + "' DAY TO SECOND";
    }

    /**
     * client runnable
     */
    private static class ClientRunnable implements Runnable {
        @Override
        public void run() {
            try {
                ServerSocket serverSocket = new ServerSocket(CLIENT_PORT);
                while (true) {
                    Socket clientSocket = serverSocket.accept();
                    DataInputStream dis = new DataInputStream(clientSocket.getInputStream());
                    DataOutputStream dos = new DataOutputStream(clientSocket.getOutputStream());
                    InetAddress clientIP = clientSocket.getInetAddress();
                    ClientHandler clientHandler = new ClientHandler(clientSocket, dis, dos);
                    int index = Server.clientVector.indexOf(new ClientData(clientIP));
                    // test mode off
                    boolean allowDuplicateClient = false;
                    ClientData clientData;
                    if (index == -1 || allowDuplicateClient) {
                        String clientName = "LAB-P115-" + String.format("%02d", clientVector.size() + 1);
                        clientData = new ClientData(clientName, clientHandler, clientIP);
                        clientVector.add(clientData);
                        new Thread(clientData.getHandler()).start();
                        dos.writeUTF("Message: connect to the server");
                    } else {
                        clientData = clientVector.get(index);
                        clientData.setHandler(clientHandler);
                        if (clientData.isOnline()) {
                            dos.writeUTF("Message: duplicate client connect");
                        } else {
                            new Thread(clientData.getHandler()).start();
                            clientData.setOnline(true);
                            dos.writeUTF("Message: connect to the server");
                        }
                    }
                }
            } catch (Exception ignored) {
            }

        }
    }

    /**
     * display runnable
     */
    private static class DisplayRunnable implements Runnable {
        @Override
        public void run() {
            try {
                ServerSocket serverSocket = new ServerSocket(DISPLAY_PORT);
                while (true) {
                    Socket displaySocket = serverSocket.accept();
                    DataInputStream dis = new DataInputStream(displaySocket.getInputStream());
                    DataOutputStream dos = new DataOutputStream(displaySocket.getOutputStream());
                    DisplayHandler displayHandler = new DisplayHandler(displaySocket, dis, dos);
                    new Thread(displayHandler).start();
                    dos.writeUTF("Message: connect to the server");
                }
            } catch (Exception ignored) {
            }
        }

    }

    /**
     * admin runnable
     */
    private static class AdminRunnable implements Runnable {
        @Override
        public void run() {
            try {
                ServerSocket serverSocket = new ServerSocket(ADMINISTRATOR_PORT);
                while (true) {
                    Socket displaySocket = serverSocket.accept();
                    DataInputStream dis = new DataInputStream(displaySocket.getInputStream());
                    DataOutputStream dos = new DataOutputStream(displaySocket.getOutputStream());
                    AdministratorHandler administratorHandler = new AdministratorHandler(displaySocket, dis, dos);
                    new Thread(administratorHandler).start();
                    dos.writeUTF("Message: connect to the server");
                }
            } catch (Exception ignored) {
            }
        }
    }

    /**
     * update runnable
     */
    private static class UpdateRunnable implements Runnable {
        @Override
        public void run() {
            while (true) {
                try {
                    String current = new SimpleDateFormat("E#yyyy-MM-dd#HH:mm").format(new Date());
                    String currentWeek = current.split("#")[0];
                    String currentDay = current.split("#")[1];
                    String currentTime = current.split("#")[2];
                    TimeUnit.SECONDS.sleep(3);
                    t.removeDuplicates();
                    // CS 261  1  2020-05-01 13:00:00  2020-05-01 15:00:00  2020-01-01 00:00:00  2020-12-31 00:00:00  Sunday
                    String[] str = t.showSubjectAndSection().split("\n");
                    for (String s : str) {
                        if (s.equals("")) {
                            currentClass = "No Class";
                            currentSection = 0;
                            continue;
                        }
                        String[] parts = s.split(" {2}");
                        boolean correctWeek = currentWeek.equals(parts[6]);
                        if (correctWeek &&
                                (parts[4].substring(0, 10)).compareTo(currentDay) <= 0 &&
                                (parts[5].substring(0, 10)).compareTo(currentDay) >= 0 &&
                                (parts[2].substring(11, 16)).compareTo(currentTime) <= 0) {
                            if ((parts[3].substring(11, 16)).compareTo(currentTime) > 0) {
                                currentClass = parts[0];
                                currentSection = Integer.parseInt(parts[1]);
                            } else if (currentSection == Integer.parseInt(parts[1]) &&
                                    currentClass.equals(parts[0]) &&
                                    (parts[3].substring(11, 16)).compareTo(currentTime) == 0) {
                                initialize();
                                currentClass = "No Class";
                                currentSection = 0;
                            }

                        }
                    }
                } catch (Exception ignored) {

                }
            }
        }
    }


}


