from django.urls import path

from .views import create_task, list_tasks, update_task_priority

urlpatterns = [
    path("create/", create_task, name="create_task"),
    path("list/", list_tasks, name="list_tasks"),
    # Add this new line for the update task priority URL
    path(
        "update_priority/<int:task_id>/",
        update_task_priority,
        name="update_task_priority",
    ),
]
