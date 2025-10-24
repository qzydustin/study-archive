// Importation of all used library and functions
import React from 'react';
import './css/single-user.css';
// Detials for people that participating in a meeting
// In the details of the meeting
// Return individuals name, email, and phone number
export function SingleUser(props){
    var user=props.user;
    return (
        <div className="single-user">
            <h5>{user.uname}</h5>
            <p>E-mail: {user.email}</p>
            <p>Phone Number: {user.phoneNumber}</p>
        </div>
    );
}