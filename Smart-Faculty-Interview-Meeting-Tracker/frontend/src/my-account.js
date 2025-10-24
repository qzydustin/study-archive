// Importation of all used library and functions
import React from 'react';
import './css/my-account.css';
import {useSelector} from 'react-redux';
// Showing the deatils of the user's credentials
export function MyAccount(){
    const user=useSelector(state=>state.user);
    // Account details from the database
    return(
        <div id="account">
            <div className = "tag">Name:</div>
            <div className="info">{user.uname}</div>
            <div className = "tag">Position:</div>
            <div className="info">{mapUserTpye(user.type)}</div>
            <div className = "tag">Contact Number:</div>
            <div className="info">{user.phoneNumber}</div>  
            <div className = "tag">Email Address:</div>
            <div className="info">{user.email}</div>
        </div>
    );
    // UserType for individual
    function mapUserTpye(type){
        if(type===0)
            return 'Super Administrator';
        if(type===1)
            return 'Department Administrator';
        if(type===2)
            return 'Administrator';
        if(type===3)
            return 'Meeting Creator';
        if(type===4)
            return 'Participant';
        return 'Candidate';
    }
}