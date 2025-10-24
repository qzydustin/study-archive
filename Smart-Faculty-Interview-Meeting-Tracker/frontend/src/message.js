// Importation of all used library and functions
import React from 'react';
import './css/message.css';
// Function for the details of the meeting
// To send message throughout the App
export function Message(props){
    const message=props.message;
    // The pattern of the message bar
    return(
        <div className="message">
            <div className="message-user">{message.u_name}</div>
            <div className="message-time">{message.postTime}</div>
            <div className="message-body">{message.m_body}</div>
        </div>
    );
}