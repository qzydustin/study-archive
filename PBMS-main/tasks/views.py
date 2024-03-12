from django.contrib.auth.decorators import login_required
from django.shortcuts import redirect, render

from .forms import TaskForm
from .models import Task

from django.shortcuts import get_object_or_404
from .forms import TaskPriorityUpdateForm

@login_required
def update_task_priority(request, task_id):
    task = get_object_or_404(Task, pk=task_id, owner=request.user)  # Ensures task belongs to user
    if request.method == 'POST':
        form = TaskPriorityUpdateForm(request.POST, instance=task)
        if form.is_valid():
            form.save()
            return redirect('list_tasks')
    else:
        form = TaskPriorityUpdateForm(instance=task)

    return render(request, 'tasks/update_task_priority.html', {'form': form, 'task': task})



@login_required
def create_task(request):
    if request.method == "POST":
        form = TaskForm(request.POST)
        if form.is_valid():
            task = form.save(commit=False)
            task.owner = request.user
            task.save()
            return redirect("list_tasks")
    else:
        form = TaskForm()
    return render(request, "create_task.html", {"form": form})


@login_required
def list_tasks(request):
    # Display only tasks owned by the logged-in user
    tasks = Task.objects.filter(owner=request.user)
    return render(request, "list_tasks.html", {"tasks": tasks})
