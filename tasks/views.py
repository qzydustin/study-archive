import json

from django.contrib.auth.decorators import login_required
from django.core import serializers
from django.http import JsonResponse
from django.shortcuts import get_object_or_404, redirect, render
from django.views.decorators.http import require_POST

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

    if request.method == "POST":
        form = TaskForm(request.POST, instance=task)
        if form.is_valid():
            form.save()
            return redirect("list_tasks")

    context = {"form": form}
    return render(request, "update_task.html", context=context)


@login_required
@require_POST
def update_task_status(request, task_id):
    data = json.loads(request.body)
    new_status = data.get("status")

    task = Task.objects.get(id=task_id)
    task.status = new_status
    task.save()

    task_json = serializers.serialize("json", [task])

    return JsonResponse({"status": "ok", "task": task_json})


# delete a task
@login_required
def delete_task(request, task_id):
    task = Task.objects.get(id=task_id)
    if request.method == "POST":
        task.delete()
        context = {"deleteConfirmation": '"' + task.title + '" was deleted!'}
        return list_tasks(request, context)

    context = {"objectTitle": task.title}
    return render(request, "delete_task.html", context=context)


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
    tasks = Task.objects.filter(owner=request.user).order_by("status", "priority")
    status_choices = Task._meta.get_field("status").choices
    tasks_by_status = {
        status: [task for task in tasks if task.status == status[0]]
        for status in status_choices
    }
    context = {"tasks_by_status": tasks_by_status}
    context.update(newContext)
    return render(request, "list_tasks.html", context=context)
