from django.urls import path

from . import views

urlpatterns = [

    # Create Task
    path('create-task', views.createTask, name="create-task"),

    # Read Task
    path('view-tasks', views.viewTasks, name="view-tasks")

]