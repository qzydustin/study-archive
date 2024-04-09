// get all task cards and columns
let taskCards = document.querySelectorAll('.task-card');
let columns = document.querySelectorAll('.column');

// add drag and drop event listeners to each task card
taskCards.forEach((taskCard) => {
    taskCard.draggable = true;
    taskCard.addEventListener('dragstart', handleDragStart);
    taskCard.addEventListener('dragend', handleDragEnd);
});

// add drag and drop event listeners to each column
columns.forEach((column) => {
    column.addEventListener('dragenter', handleDragEnter);
    column.addEventListener('dragleave', handleDragLeave);
    column.addEventListener('dragover', handleDragOver);
    column.addEventListener('drop', handleDrop);
});

// drag and drop event handlers
function handleDragStart(e) {
    e.dataTransfer.setData('text/plain', e.target.id);
}

function handleDragEnd(e) {
    e.target.style.opacity = '1';
}

function handleDragEnter(e) {
    e.preventDefault();
    e.target.classList.add('over');
}

function handleDragLeave(e) {
    e.target.classList.remove('over');
}

function handleDragOver(e) {
    e.preventDefault();
}

function handleDrop(e) {
    // only drop task cards into column elements or their children
    let target = e.target;
    while (target !== null && !target.classList.contains('column')) {
        target = target.parentNode;
    }
    if (target === null) {
        return;
    }
    e.preventDefault();
    let id = e.dataTransfer.getData('text/plain');
    let draggableElement = document.getElementById(id);
    target.appendChild(draggableElement);
    target.classList.remove('over');
    let newStatus = target.getAttribute('id');

    // send Fetch request
    let csrftoken = getCookie('csrftoken');

    fetch(`/tasks/update_task_status/${id}/`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'X-CSRFToken': csrftoken
        },
        body: JSON.stringify({ status: newStatus })
    });
}

function getCookie(name) {
    let cookieValue = null;
    if (document.cookie && document.cookie !== '') {
        let cookies = document.cookie.split(';');
        for (let i = 0; i < cookies.length; i++) {
            let cookie = cookies[i].trim();
            if (cookie.substring(0, name.length + 1) === (name + '=')) {
                cookieValue = decodeURIComponent(cookie.substring(name.length + 1));
                break;
            }
        }
    }
    return cookieValue;
}
