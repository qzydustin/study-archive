from django import forms

from .models import Task


class TaskForm(forms.ModelForm):
    class Meta:
        model = Task
        fields = ["title", "content", "priority", "status", "due_date"]


# Add a new form for updating the task's priority
class TaskPriorityUpdateForm(forms.ModelForm):
    class Meta:
        model = Task
        fields = ["priority"]
