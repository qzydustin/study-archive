from django.shortcuts import render, redirect
from .models import Task
from .forms import TaskForm
# Create your views here.

def create_task(request):
    if request.method == 'POST':
        form = TaskForm(request.POST)
        if form.is_valid():
            form.save()
            # TODO: Redirect to the tasks list
            return redirect('home')
    else:
        form = TaskForm()
    return render(request, 'create_task.html', {'form': form})

# read a task
def viewTasks(request):
   tasks = Task.objects.all()

   context = {'tasks' : tasks}

   return render(request, 'view-tasks.html', context=context)