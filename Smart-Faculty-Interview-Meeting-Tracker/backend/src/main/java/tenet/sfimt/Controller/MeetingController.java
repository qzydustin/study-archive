package tenet.sfimt.Controller;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import tenet.sfimt.Model.Location;
import tenet.sfimt.Model.Meeting;
import tenet.sfimt.Model.Message;
import tenet.sfimt.Model.User;
import tenet.sfimt.Service.MeetingService;

import java.util.List;

@RestController
public class MeetingController {

    @Autowired
    MeetingService service;

    @GetMapping(value = "/getUser/{email}/{u_password}")
    public User getUser(@PathVariable("email") String email, @PathVariable("u_password") String password) {
        return service.getUser(email, password);
    }

    @GetMapping(value = "/getAllMeetings")
    public List<Meeting> getAllMeetings() {
        return service.getAllMeetings();
    }

    @GetMapping(value = "/getMeetingsByUserId/{u_id}")
    public List<Meeting> getMeetingsByUserId(@PathVariable("u_id") int id) {
        return service.getMeetingsByUserId(id);
    }

    @GetMapping(value = "/getCandidate")
    public List<User> getCandidate() {
        return service.getCandidate();
    }

    @GetMapping(value = "/getParticipant")
    public List<User> getParticipant() {
        return service.getParticipant();
    }

    @GetMapping(value = "/getLocation")
    public List<Location> getLocation() {
        return service.getLocation();
    }

    @GetMapping(value = "/getMessage/{id}")
    public List<Message> getMessage(@PathVariable("id") int id) {
        return service.getMessage(id);
    }

    @PostMapping(value = "/insertMeeting")
    public void insertMeeting(@RequestBody Meeting meeting) {
        service.insertMeeting(meeting);
    }

    @PostMapping(value = "/insertUser")
    public void insertUser(@RequestBody User user) {
        service.insertUser(user);
    }

    @PostMapping(value = "/insertMessage")
    public Message insertMessage(@RequestBody Message message) {
        return service.insertMessage(message);
    }

    @DeleteMapping(value = "/deleteMeeting/{m_id}")
    public void deleteMeeting(@PathVariable("m_id") int id) {
        service.deleteMeeting(id);
    }

    @PutMapping(value = "/addFeedback")
    public void addFeedback(@RequestBody Meeting meeting) {
        service.addFeedback(meeting);
    }
}

