package tenet.sfimt.Model;

import com.fasterxml.jackson.annotation.JsonFormat;

import java.sql.Timestamp;

public class Message {
    private int m_id;
    private int b_id;
    private String u_name;
    private String m_body;
    @JsonFormat(pattern = "yyyy-MM-dd HH:mm:ss")
    private Timestamp postTime;

    public int getM_id() {
        return m_id;
    }

    public void setM_id(int m_id) {
        this.m_id = m_id;
    }

    public int getB_id() {
        return b_id;
    }

    public void setB_id(int b_id) {
        this.b_id = b_id;
    }

    public String getU_name() {
        return u_name;
    }

    public void setU_name(String u_name) {
        this.u_name = u_name;
    }

    public String getM_body() {
        return m_body;
    }

    public void setM_body(String m_body) {
        this.m_body = m_body;
    }

    public Timestamp getPostTime() {
        return postTime;
    }

    public void setPostTime(Timestamp postTime) {
        this.postTime = postTime;
    }
}
