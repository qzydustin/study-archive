from django.urls import path

from .views import create_task, viewTasks

urlpatterns = [
    path('create/', create_task, name='create-task'),
    path('view-tasks', viewTasks, name="view-tasks"),
]