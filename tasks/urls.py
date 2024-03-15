from django.urls import path
from . import views

urlpatterns = [
    path("create/", views.create_task, name="create_task"),
    path("list/", views.list_tasks, name="list_tasks"),
    # Add this new line for the update task priority URL
    path("update_priority/<int:task_id>/", views.update_task_priority, name="update_task_priority"),

    # Update Task
    # str - string, pk - taskID
    path('update-task/<int:task_id>/', views.update_task, name="update_task"),

    # Delete Task
    path('delete-task/<int:task_id>/', views.delete_task, name="delete_task"),

]

