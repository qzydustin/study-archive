from django.contrib.auth.decorators import login_required
from django.shortcuts import get_object_or_404, redirect, render

from .forms import TaskForm, TaskPriorityUpdateForm
from .models import Task


@login_required
def update_task_priority(request, task_id):
    task = get_object_or_404(
        Task, pk=task_id, owner=request.user
    )  # Ensures task belongs to user
    if request.method == "POST":
        form = TaskPriorityUpdateForm(request.POST, instance=task)
        if form.is_valid():
            form.save()
            return redirect("list_tasks")
    else:
        form = TaskPriorityUpdateForm(instance=task)

    return render(
        request, "tasks/update_task_priority.html", {"form": form, "task": task}
    )

# update a task
@login_required
def update_task(request, task_id):
   task = Task.objects.get(id=task_id)
   form = TaskForm(instance=task)

   if request.method == 'POST':
      form = TaskForm(request.POST, instance = task)
      if form.is_valid():
         form.save()
         return redirect('list_tasks')

   context = {'form' : form}
   return render(request, 'update_task.html', context=context)

# delete a task
@login_required
def delete_task(request, task_id):
   task = Task.objects.get(id=task_id)
   if request.method == 'POST':
      task.delete()
      context = {'deleteConfirmation' : '"' + task.title + '" was deleted!'}
      return list_tasks(request,context)

   context = {'objectTitle':task.title}
   return render(request, 'delete_task.html', context=context)

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
def list_tasks(request, newContext={}):
    # Display only tasks owned by the logged-in user
    tasks = Task.objects.filter(owner=request.user)
    context = {'tasks' : tasks}
    context.update(newContext)
    return render(request, "list_tasks.html", context=context)
