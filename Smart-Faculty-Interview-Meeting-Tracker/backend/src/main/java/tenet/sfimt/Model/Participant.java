package tenet.sfimt.Model;

import java.sql.Date;
import java.sql.Time;

public class Participant extends User {
    private Date availableStartDate;
    private Time availableStartHour;
    private Date availableEndDate;
    private Time availableEndHour;

    public Time getAvailableStartHour() {
        return availableStartHour;
    }

    public void setAvailableStartHour(Time availableStartHour) {
        this.availableStartHour = availableStartHour;
    }

    public Date getAvailableStartDate() {
        return availableStartDate;
    }

    public void setAvailableStartDate(Date availableStartDate) {
        this.availableStartDate = availableStartDate;
    }

    public Date getAvailableEndDate() {
        return availableEndDate;
    }

    public void setAvailableEndDate(Date availableEndDate) {
        this.availableEndDate = availableEndDate;
    }

    public Time getAvailableEndHour() {
        return availableEndHour;
    }

    public void setAvailableEndHour(Time availableEndHour) {
        this.availableEndHour = availableEndHour;
    }
}
