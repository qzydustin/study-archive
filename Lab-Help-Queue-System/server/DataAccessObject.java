package server;

import java.sql.*;

/**
 * @author qiz9744, wangs4830
 */
public class DataAccessObject {

    private Connection daoConn = null;
    private ResultSet daoRset = null;

    /**
     * connect to the Oracle database
     */
    public void connect(String user, String pass) throws SQLException, ClassNotFoundException {

        // --- 1) get the Class object for the driver
        try {
            Class.forName("oracle.jdbc.OracleDriver");
        } catch (ClassNotFoundException e) {
            System.err.println("Could not get class object for Driver");
        }

        // --- 2) connect to database
//        try {
        daoConn = DriverManager.getConnection(
                "jdbc:oracle:thin:@alfred.cs.uwec.edu:1521:csdev", user, pass);
//        } catch (SQLException sqlException) {
//            System.err.println("Could not make connection to database");
//            System.err.println(sqlException.getMessage());
//        }
    }

    /**
     * execute an SQL query
     *
     * @param sqlQuery SQL query
     * @return oracle return message
     */
    public String executeSQLQuery(String sqlQuery) throws SQLException {

        Statement stmt = null;
        daoRset = null;

//        try {
        stmt = daoConn.createStatement();
        daoRset = stmt.executeQuery(sqlQuery);
//        } catch (SQLException sqlException) {
//            System.err.println("Could not execute SQL statement: >" + sqlQuery + "<");
//            System.err.println(sqlException.getMessage());
//            // rollback
//            rollback();
//        }

        ResultSetMetaData rsmd = null;
        int columnCount = -1;
        StringBuilder resultString = new StringBuilder();

        try {
            rsmd = daoRset.getMetaData();
            // get number of columns from result set metadata
            columnCount = rsmd.getColumnCount();
            // row processing of result set
            while (daoRset.next()) {
                for (int index = 1; index <= columnCount; index++) {
                    resultString.append(daoRset.getString(index)).append("  ");
                }
                resultString.append("\n");
            }
            assert stmt != null;
            stmt.close();
        } catch (SQLException sqlException) {
            System.err.println("Error in processing result set");
            System.err.println(sqlException.getMessage());
        } catch (NullPointerException npe) {
            System.err.println("DAO, processResultSet() - no result set generated");
            System.err.println(npe.getMessage());
        }
        return resultString.toString();
    }

    /**
     * execute an SQL command that is not a query
     *
     * @param sqlCommand sql command
     * @return oracle return message
     */
    public int executeSQLNonQuery(String sqlCommand) throws SQLException {
        Statement stmt = null;
        int returnValue = -1;
//        try {
        stmt = daoConn.createStatement();
        returnValue = stmt.executeUpdate(sqlCommand);
        stmt.close();
//        } catch (SQLException sqlException) {
//            System.err.println("Could not execute SQL command: >" + sqlCommand + "<");
//            System.err.println("Return value: " + returnValue);
//            System.err.println(sqlException.getMessage());
//            // rollback
//            rollback();
//        }
        return returnValue;
    }


    /**
     * set autocommit on or off based on flag
     *
     * @param flag true or false
     */
    public void setAutoCommit(boolean flag) {
        try {
            daoConn.setAutoCommit(flag);
        } catch (SQLException sqlException) {
            System.err.println("DAO, setAutoCommit() - error in setting");
            System.err.println(sqlException.getMessage());
        }
    }

    /**
     * commit current transaction on connection
     */
    public void commit() {
        try {
            daoConn.commit();
        } catch (SQLException sqlException) {
            System.err.println("DAO, commit() - error in commit");
            System.err.println(sqlException.getMessage());
        }
    }

    /**
     * rollback current transaction on connection
     */
    public void rollback() {
        try {
            daoConn.rollback();
        } catch (SQLException sqlException) {
            System.err.println("DAO, rollback() - error in rollback");
            System.err.println(sqlException.getMessage());
        }
    }

    /**
     * disconnect from the Oracle database
     */
    public void disconnect() {
        try {
            if (daoConn != null) {
                daoConn.close();
            }
            if (daoRset != null) {
                daoRset = null;
            }
        } catch (SQLException sqlException) {
            System.err.println("Error in closing database connection");
            System.err.println(sqlException.getMessage());
        } finally {
            if (daoConn != null) {
                try {
                    daoConn.close();
                } catch (SQLException sqlException) {
                    daoConn = null;
                }
            }
            if (daoRset != null) {
                daoRset = null;
            }
        }
    }


}