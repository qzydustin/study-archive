from django.urls import path

from .views import create_task, list_tasks

urlpatterns = [
    path("create/", create_task, name="create_task"),
    path("list/", list_tasks, name="list_tasks"),
]
