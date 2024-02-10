from django.shortcuts import render, redirect
from .models import Task
from .forms import TaskForm
# Create your views here.


# create a task
def createTask(request):
   form = TaskForm()

   if request.method == 'POST':
      form = TaskForm(request.POST)

      if form.is_valid():
         form.save()
         # return HttpResponse('Your task was created')
         return redirect('view-tasks')

   context = {'form' : form}

   return render(request, 'create-task.html', context=context)

# read a task
def viewTasks(request):
   tasks = Task.objects.all()

   context = {'tasks' : tasks}

   return render(request, 'view-tasks.html', context=context)