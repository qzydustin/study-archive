package tenet.sfimt.Service;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import tenet.sfimt.DAO.MeetingDAO;
import tenet.sfimt.Model.*;

import java.util.List;

@Service
public class MeetingService {

    @Autowired
    MeetingDAO dao;
    @Autowired
    EmailService emailService;

    public User getUser(String email, String password) {
        return dao.findUser(email, password);
    }

    public List<Meeting> getAllMeetings() {
        List<Meeting> meetings = dao.findAllMeetings();
        for (int i = 0; i < meetings.size(); i++) {
            Meeting meeting = meetings.get(i);
            meeting.setUserList(dao.findRelatedUsers(meeting.getMId()));
            Location location = dao.findLocation(meeting.getLId());
            meeting.setAddress(location.getAddress());
            meeting.setCity(location.getCity());
        }
        return meetings;
    }

    public List<Meeting> getMeetingsByUserId(int id) {
        List<Meeting> meetings = dao.findMeetingsByUserId(id);
        for (int i = 0; i < meetings.size(); i++) {
            Meeting meeting = meetings.get(i);
            meeting.setUserList(dao.findRelatedUsers(meeting.getMId()));
            Location location = dao.findLocation(meeting.getLId());
            meeting.setAddress(location.getAddress());
            meeting.setCity(location.getCity());
        }
        return meetings;
    }

    public List<User> getCandidate() {
        return dao.findCandidate();
    }

    public List<User> getParticipant() {
        return dao.findParticipant();
    }

    public List<Location> getLocation() {
        return dao.findLocation();
    }

    public void insertMeeting(Meeting meeting) {
        int mId = dao.addMeeting(meeting);
        List<UserMeeting> userList = meeting.getUserList();
        for (int i = 0; i < userList.size(); i++) {
            dao.addUserMeeting(mId, userList.get(i).getUId());
            Email e = new Email();
            e.setTo(userList.get(i).getEmail());
            e.setFrom("createMeeting@qzydustin.tk");
            e.setSubject("A new meeting related to you is created by UWEC administration");
            e.setText("A meeting is created by UWEC administration, please see more detail on https://interviewtracker.qzydustin.tk/");
//            Email e2 = new Email();
//            // default remind before 10 min
//            int reminderMin = 10;
//            TimeZone.setDefault(TimeZone. getTimeZone("UTC"));
//            e2.setSentDate(new Timestamp(meeting.getStartTime().getTime() - (reminderMin * 60 * 1000L)));
//            e2.setTo(userList.get(i).getEmail());
//            e2.setFrom("meetingReminder@qzydustin.tk");
//            e2.setSubject("A meeting will start in 10 min");
//            e2.setText("A meeting will start in 10 min, please see more detail on https://interviewtracker.qzydustin.tk/");
            emailService.sendMail(e);
//            emailService.sendMail(e2);
        }
    }

    public Message insertMessage(Message message) {
        return dao.addMessage(message);
    }

    public List<Message> getMessage(int id) {
        return dao.findMessage(id);
    }

    public void insertUser(User user) {
        dao.addUser(user);
        Email e = new Email();
        e.setTo(user.getEmail());
        e.setFrom("createAccount@qzydustin.tk");
        e.setSubject("An account related to you is created by UWEC administration");
        e.setText("An account related to you is created by UWEC administration, please see more detail on https://interviewtracker.qzydustin.tk/");
        emailService.sendMail(e);
    }

    public void deleteMeeting(int id) {
        dao.deleteAllUsersFromMeeting(id);
        dao.deleteMessage(id);
        dao.deleteMeeting(id);
    }

    public void addFeedback(Meeting meeting) {
        dao.updateFeedback(meeting);
    }
}
