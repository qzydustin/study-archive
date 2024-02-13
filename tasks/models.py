from django.db import models
from django.contrib.auth.models import User


class Task(models.Model):
    # Retain the original title and content fields
    title = models.CharField(max_length=85)
    content = models.CharField(max_length=300)

    # New fields
    # Task priority, integer type, default value is 1
    priority = models.IntegerField(default=1)

    # Task status, with predefined choices
    status = models.CharField(
        max_length=25,
        choices=[("todo", "To Do"), ("in_progress", "In Progress"), ("done", "Done")],
        default="todo",
    )

    # Task due date, can be null
    due_date = models.DateField(null=True, blank=True)

    # Posting date, automatically set to current time
    posted_date = models.DateTimeField(auto_now_add=True)

    # User association
    owner = models.ForeignKey(User, on_delete=models.CASCADE, related_name="tasks")

    def __str__(self):
        return self.title
