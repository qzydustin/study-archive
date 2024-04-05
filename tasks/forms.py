from django import forms
from django.utils.safestring import SafeString

from .models import Task


class TaskForm(forms.ModelForm):
    # use "form-group" as_div class for styling
    def as_div(self):
        return SafeString(super().as_div().replace("<div>", "<div class='form-group'>"))

    class Meta:
        model = Task
        fields = ["title", "content", "priority", "status", "due_date"]


# Add a new form for updating the task's priority
class TaskPriorityUpdateForm(forms.ModelForm):
    class Meta:
        model = Task
        fields = ["priority"]
