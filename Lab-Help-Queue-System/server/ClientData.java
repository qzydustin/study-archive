package server;

import java.net.InetAddress;
import java.util.Date;

/**
 * @author qiz9744, wangs4830
 */
public class ClientData {
    private String name;
    private ClientHandler handler;
    private boolean online = true;
    private boolean helping = false;
    private Date date;
    private String ip;

    /**
     * constructor
     */
    public ClientData(String name, ClientHandler handler, InetAddress ip) {
        this.name = name;
        this.handler = handler;
        this.ip = ip.getHostName();
    }

    public ClientData(String name) {
        this.name = name;
    }

    public ClientData(InetAddress ip) {
        this.ip = ip.getHostName();
    }

    public ClientData(ClientHandler handler) {
        this.handler = handler;
    }

    public String getName() {
        return name;
    }

    public ClientHandler getHandler() {
        return handler;
    }

    public void setHandler(ClientHandler handler) {
        this.handler = handler;
    }

    public boolean isOnline() {
        return online;
    }

    public void setOnline(boolean online) {
        this.online = online;
    }

    public boolean isHelping() {
        return helping;
    }

    public void setHelping(boolean helping) {
        this.helping = helping;
    }

    public Date getDate() {
        return date;
    }

    public void setDate(Date date) {
        this.date = date;
    }

    /**
     * override equals to look for clientData from clientVector
     *
     * @param obj content
     * @return true or false
     */
    @Override
    public boolean equals(Object obj) {
        if (obj.getClass() == ClientData.class) {
            ClientData o = (ClientData) obj;
            if (ip != null) {
                return ip.equals(o.ip);
            } else if (handler != null) {
                return handler.equals(o.handler);
            } else if (name != null) {
                return name.equals(o.name);
            }
        }
        return super.equals(obj);
    }

}
