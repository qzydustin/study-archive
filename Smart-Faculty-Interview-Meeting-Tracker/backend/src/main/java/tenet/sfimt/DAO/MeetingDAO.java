package tenet.sfimt.DAO;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.BeanPropertyRowMapper;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.support.GeneratedKeyHolder;
import org.springframework.stereotype.Repository;
import tenet.sfimt.Model.*;

import java.sql.PreparedStatement;
import java.util.List;

@Repository
public class MeetingDAO {

    @Autowired
    JdbcTemplate jdbcTemplate;

    public User findUser(String email, String password) {
        String sql = "select * from users where email='" + email + "' and u_password='" + password + "'";
        List<User> user = jdbcTemplate.query(sql, new BeanPropertyRowMapper(User.class));
        if (user.size() != 0 && user.get(0).getUPassword().equals(password)) {
            return user.get(0);
        } else {
            return new User();
        }
    }

    public List<Meeting> findAllMeetings() {
        String sql = "select * from meeting";
        return jdbcTemplate.query(sql, new BeanPropertyRowMapper(Meeting.class));
    }

    public List<Meeting> findMeetingsByUserId(int id) {
        String sql = "select * from meeting where m_id=any(select b_id from user_meeting where u_id=" + id + ")";
        return jdbcTemplate.query(sql, new BeanPropertyRowMapper<>(Meeting.class));
    }

    public Location findLocation(int id) {
        String sql = "select * from location where l_id=" + id;
        return (Location) jdbcTemplate.query(sql, new BeanPropertyRowMapper(Location.class)).get(0);
    }

    public List<UserMeeting> findRelatedUsers(int id) {
        String sql = "select u_id,email,phone_number,u_name,type from users where u_id=any(select u_id from user_meeting where b_id=" + id + ")";
        return jdbcTemplate.query(sql, new BeanPropertyRowMapper<>(UserMeeting.class));
    }

    public List<User> findCandidate() {
        String sql = "select * from users where type=5";
        return jdbcTemplate.query(sql, new BeanPropertyRowMapper<>(User.class));
    }

    public List<User> findParticipant() {
        String sql = "select * from users where type<5 and type>1";
        return jdbcTemplate.query(sql, new BeanPropertyRowMapper<>(User.class));
    }

    public List<Message> findMessage(int id) {
        String sql = "select * from message where b_id=" + id + " order by post_time";
        return jdbcTemplate.query(sql, new BeanPropertyRowMapper<>(Message.class));
    }

    public List<Location> findLocation() {
        String sql = "select * from location";
        return jdbcTemplate.query(sql, new BeanPropertyRowMapper<>(Location.class));
    }

    public int addMeeting(Meeting meeting) {
        GeneratedKeyHolder keyHolder = new GeneratedKeyHolder();
        String sql = "insert into meeting values(null,?,?,?,?,?)";
        jdbcTemplate.update(connection -> {
            PreparedStatement ps = connection.prepareStatement(sql, new String[]{"m_id"});
            ps.setTimestamp(1, meeting.getStartTime());
            ps.setTimestamp(2, meeting.getEndTime());
            ps.setInt(3, meeting.getLId());
            ps.setString(4, meeting.getFeedback());
            ps.setInt(5, meeting.getStatus());
            return ps;
        }, keyHolder);
        return keyHolder.getKey().intValue();
    }

    public void addUserMeeting(int mId, int uId) {
        String sql = "insert into user_meeting values(" + uId + "," + mId + ")";
        jdbcTemplate.update(sql);
    }

    public void addUser(User user) {
        String sql = "insert into users values(null,?,?,?,?,?)";
        Object[] params = {user.getEmail(), user.getUPassword(), user.getPhoneNumber(), user.getUName(), user.getType()};
        jdbcTemplate.update(sql, params);
    }

    public Message addMessage(Message message) {
        GeneratedKeyHolder keyHolder = new GeneratedKeyHolder();
        String sql = "insert into message values(?,null,?,?,?)";
        jdbcTemplate.update(connection -> {
            PreparedStatement ps = connection.prepareStatement(sql, new String[]{"m_id"});
            ps.setInt(1, message.getB_id());
            ps.setString(2, message.getU_name());
            ps.setString(3, message.getM_body());
            ps.setTimestamp(4, message.getPostTime());
            return ps;
        }, keyHolder);
        message.setM_id(keyHolder.getKey().intValue());
        return message;
    }

    public void deleteUserFromMeeting(int u_id, int b_id) {
        String sql = "delete from user_meeting where u_id=" + u_id + " and b_id=" + b_id;
        jdbcTemplate.update(sql);
    }

    public void deleteAllUsersFromMeeting(int b_id) {
        String sql = "delete from user_meeting where b_id=" + b_id;
        jdbcTemplate.update(sql);
    }

    public void deleteMessage(int b_id) {
        String sql = "delete from message where b_id=" + b_id;
        jdbcTemplate.update(sql);
    }

    public void deleteMeeting(int m_id) {
        String sql = "delete from meeting where m_id=" + m_id;
        jdbcTemplate.update(sql);
    }

    public void updateFeedback(Meeting meeting) {
        String sql = "update meeting set feedback='" + meeting.getFeedback() + "' where m_id=" + meeting.getMId();
        jdbcTemplate.update(sql);
    }

}
