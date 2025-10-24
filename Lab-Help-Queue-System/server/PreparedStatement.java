package server;

import java.sql.SQLException;

/**
 * @author qiz9744, wangs4830
 */
public class PreparedStatement {
    private final DataAccessObject dao;

    /**
     * constructor
     *
     * @param dao DAO
     */
    public PreparedStatement(DataAccessObject dao) {
        this.dao = dao;
    }


    public void initializeEvent() throws SQLException {
        dao.executeSQLNonQuery("DROP TABLE Events");
        dao.executeSQLNonQuery("CREATE TABLE Events\n" +
                "(\n" +
                "    e_id            number,\n" +
                "    e_type          varchar(10),\n" +
                "    e_ws_name       varchar(15),\n" +
                "    e_role          varchar(10),\n" +
                "    e_class         varchar(10),\n" +
                "    e_class_section number(1),\n" +
                "    e_date          date,\n" +
                "    e_wait_time     INTERVAL DAY TO SECOND,\n" +
                "    PRIMARY KEY (e_id)\n" +
                ")");
        dao.executeSQLNonQuery("DROP SEQUENCE ID");
        dao.executeSQLNonQuery("CREATE SEQUENCE ID\n" +
                "    MINVALUE 1 NOMAXVALUE\n" +
                "    INCREMENT BY 1\n" +
                "    START\n" +
                "        WITH 1 NOCACHE");
        commit();
    }

    public void initializeSubject() throws SQLException {
        dao.executeSQLNonQuery("DROP TABLE Subject");
        dao.executeSQLNonQuery("CREATE TABLE Subject\n" +
                "(\n" +
                "    s_subject      varchar(6),\n" +
                "    s_section    number(1),\n" +
                "    s_startTime  date,\n" +
                "    s_endTime    date,\n" +
                "    s_startDate  date,\n" +
                "    s_endDate    date,\n" +
                "    s_week       varchar(3)\n CHECK(s_week IN ('Mon','Tue','Wed','Thu','Fri','Sat','Sun'))" +
                ")");
        commit();
    }


    /**
     * insert subject
     *
     * @param subject   subject
     * @param section   section
     * @param startTime start
     * @param endTime   end
     * @param startDate start
     * @param endDate   end
     * @param week      week
     */
    public void insertSubject(String subject, int section, String startTime, String endTime,
                              String startDate, String endDate, String week) throws SQLException {
        dao.executeSQLNonQuery("INSERT INTO Subject VALUES (" +
                "'" + subject + "'," +
                +section + "," +
                "to_date('" + startTime + "', 'HH24:MI:SS')," +
                "to_date('" + endTime + "', 'HH24:MI:SS')," +
                "to_date('" + startDate + "', 'YYYY-MM-DD')," +
                "to_date('" + endDate + "', 'YYYY-MM-DD')," +
                "'" + week + "')"
        );
        commit();
    }

    /**
     * insert event
     *
     * @param type     type
     * @param name     name
     * @param role     role
     * @param subject  subject
     * @param section  section
     * @param date     date
     * @param waitTime wait time
     */
    public void insertEvent(String type, String name, String role, String subject,
                            int section, String date, String waitTime) throws SQLException {
        dao.executeSQLNonQuery("INSERT INTO EVENTS VALUES (" +
                "ID.NEXTVAL, " +
                "'" + type + "', " +
                "'" + name + "', " +
                "'" + role + "', " +
                "'" + subject + "', " +
                "'" + section + "', " +
                "to_date('" + date + "', 'YYYY-MM-DD HH24:MI:SS')," +
                "" + waitTime + ")"
        );
        commit();
    }

    /**
     * remove duplicates
     */
    public void removeDuplicates() throws SQLException {
        dao.executeSQLNonQuery("DELETE\n" +
                "FROM subject\n" +
                "WHERE rowid not in\n" +
                "      (SELECT MIN(rowid)\n" +
                "       FROM subject\n" +
                "       GROUP BY S_SUBJECT, S_SECTION, S_STARTTIME, S_ENDTIME, S_STARTDATE, S_ENDDATE, S_WEEK)");
        commit();
    }

    /**
     * show subject and section
     *
     * @return content
     */
    public String showSubjectAndSection() throws SQLException {
        return dao.executeSQLQuery("select *\n" +
                "from Subject\n");
    }


    /**
     * database commit
     */
    private void commit() throws SQLException {
        dao.executeSQLNonQuery("commit");
    }

    /**
     * log in
     *
     * @param username username
     * @return
     * @throws SQLException
     */
    public String login(String username) throws SQLException {
        return dao.executeSQLQuery("SELECT a_password FROM adminAccount\n" +
                "WHERE a_username = '" + username + "'");
    }
}
