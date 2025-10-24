// Importation of all used library and functions
import React from 'react';
import {MeetingView} from './meeting-view.js';
import './css/meeting-tracker.css';
import {useDispatch, useSelector} from 'react-redux';
import {changeView} from './actions.js';
// Function for briefly elaborate the details on the meetings
export function MeetingTracker(props){
    var info = props.info;
    const currentView=useSelector(state=>state.currentView);
    const dispatch=useDispatch();
    return(
        <div className="Container_Info">
            <div onClick={()=>openView(info.mid)}>
                <p>{parseTime(info.startTime)+'\xa0\xa0\xa0\xa0~\xa0\xa0\xa0\xa0'+parseTime(info.endTime)}</p>
                <p>{info.address+', '+info.city}</p>
            </div>
            {currentView===info.mid&&<MeetingView key={info.mid} info={info}/>}
        </div>
        
    );
    // Open the details of the meeting
    function openView(id){
        dispatch(changeView(id));
    }
    // show the start time
    function parseTime(time){
        var result=time.substring(5,7)+'/'+time.substring(8,10)+'/'+time.substring(0,4)+'\xa0\xa0\xa0'+time.substring(11,19);
        return result;
    }
}