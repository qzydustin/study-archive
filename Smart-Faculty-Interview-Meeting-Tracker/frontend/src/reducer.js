// Importation of all used library and functions
import {Action} from './actions.js';
// Define an initial state value for the app 
const intialState={
    currentPage:0,
    currentView:0,
    currentOption:0,
    meetings:[],
    user:{},
    loginError:false,
    candidate:[],
    participant:[],
    location:[],
    message:[],
    feedbackOption:0
};         
// Create a "reducer" funtion that determines the new state
function reducer(state=intialState,action){
    // look for the type of action has occured
    // decide how to update the state
    switch(action.type){
        // Action handling for loadMeetings
        case Action.loadMeetings:
            return{
                ...state,
                meetings:action.payload,
            };
        // Action handling for loadUser
        case Action.loadUser:
            return{
                ...state,
                user:action.payload,
                currentPage:1,
            };
        // Action handling for ShowLoginError
        case Action.showLoginError:
            return{
                ...state,
                loginError:true,
            };
         // Action handling for changeView
        case Action.changeView:
            return{
                ...state,
                currentView:action.payload,
            };
        // Action handling for userLogout    
        case Action.userLogout:
            return{
                ...state,
                currentPage:0,
                user:{},
                currentOption:0,
            };
        // Action handling for changeOption    
        case Action.changeOption:
            return{
                ...state,
                currentOption:action.payload,
            };
        // Action handling for changePage    
        case Action.changePage:
            return{
                ...state,
                currentPage:action.payload,
            };
        // Action handling for loadCandidate    
        case Action.loadCandidate:
            return{
                ...state,
                candidate:action.payload,
            };
        // Action handling for loadParticipant    
        case Action.loadParticipant:
            return{
                ...state,
                participant:action.payload,
            };
        // Action handling for loadLocation    
        case Action.loadLocation:
            return{
                ...state,
                location:action.payload,
            };
        // Action handling for loadMessage    
        case Action.loadMessage:
            return{
                ...state,
                message:action.payload,
            };
        // Action handling for removeMeeting
        case Action.removeMeeting:
            var temp=state.meetings.filter(meeting=>meeting.mid!==action.payload);
            return{
                ...state,
                meetings:temp,
            };
        // Action handling for insertMessage
        case Action.insertMessage:
            var message=state.message.slice(0);
            message.push(action.payload);
            return{
                ...state,
                message:message,
            };
        // Action handling for insertFeedback
        case Action.insertFeedback:
            var meetings=state.meetings.map(meeting=>{
                if(meeting.mid===action.payload.mid){
                    meeting.feedback=action.payload.feedback;
                }
                // no relavant meeting id found then return to orignal state
                return meeting;
            });
            return{
                ...state,
                meetings:meetings,
            };
        // Action handling for changeFeedbackOption
        case Action.changeFeedbackOption:
            return{
                ...state,
                feedbackOption:action.payload,
            };
        // if no relavant action type occured
        // return to the original state 
        default:
            return state;
    }
}

export default reducer;