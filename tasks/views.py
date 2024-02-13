from django.shortcuts import render, redirect
from .models import Task
from .forms import TaskForm


def create_task(request):
    if request.method == 'POST':
        form = TaskForm(request.POST)
        if form.is_valid():
            form.save()
            return redirect('list_tasks')
    else:
        form = TaskForm()
    return render(request, 'create_task.html', {'form': form})


def list_tasks(request):
    # Retrieve all tasks from the database
    tasks = Task.objects.all()
    return render(request, 'list_tasks.html', {'tasks': tasks})
