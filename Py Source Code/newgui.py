import ttkbootstrap as tk
from tkinter import filedialog
from ctypes import *
import os

# 定义函数以执行 DLL 中的 exec_0 函数
def exe_0():
    dir_py_str = dir_path_str_var.get()
    result = lib.ScanOnly(dir_py_str.encode('utf-8'))
    if result == 0:
        myout.insert(tk.END, "生成的文本文件在{}\\allfile中\n".format(parent_directory))
    else:
        myout.insert(tk.END, "分析失败\n")

# 定义函数以执行 DLL 中的 exec_1 函数
def exe_1():
    dir_py_str = dir_path_str_var.get()
    mystat_py_str = mystat_path_str_var.get()
    result = lib.ScanAndStats(dir_py_str.encode('utf-8'), mystat_py_str.encode('utf-8'))
    if result == 0:
        myout.insert(tk.END, "生成的文本文件在{}\\allfile中\n".format(parent_directory))
    else:
        myout.insert(tk.END, "分析失败\n")

# 定义函数以执行 DLL 中的 exec_2 函数
def exe_2():
    dir_py_str = dir_path_str_var.get()
    mystat_py_str = mystat_path_str_var.get()
    mydir_py_str = mydir_path_str_var.get()
    result = lib.ScanAndStatsAndDelete(dir_py_str.encode('utf-8'), mystat_py_str.encode('utf-8'), mydir_py_str.encode('utf-8'))
    if result == 0:
        myout.insert(tk.END, "生成的文本文件在{}\\allfile中\n".format(parent_directory))
    else:
        myout.insert(tk.END, "分析失败\n")


# 定义函数以执行 DLL 中的 exec_3 函数0
def exe_3():
    dir_py_str = dir_path_str_var.get()
    mystat_py_str = mystat_path_str_var.get()
    mydir_py_str = mydir_path_str_var.get()
    myfile_py_str = myfile_path_str_var.get()
    result = lib.ScanAndStatsAndDeleteFile(dir_py_str.encode('utf-8'), mystat_py_str.encode('utf-8'), mydir_py_str.encode('utf-8'),
                        myfile_py_str.encode('utf-8'))
    if result == 0:
        myout.insert(tk.END, "生成的文本文件在{}\\allfile中\n".format(parent_directory))
    else:
        myout.insert(tk.END, "分析失败\n")

# 当 Radiobutton 被选中时触发的事件处理函数
def on_radiobutton_select():
    # 当Radiobutton被选中时触发
    value = type_int_var.get()
    update_layout(value)

# 定义函数以打开文件选择对话框并设置 mystat_path_str_var
def choose_mystat():
    # 使用filedialog打开文件选择对话框，并获取选中的文件路径
    global mystat_path_str_var
    mystat_path_str_var.set(filedialog.askopenfilename().replace("/","\\"))
    if mystat_path_str_var:
        mystat_path_str_var.get()  # 或者执行其他你需要的操作
    else:
        print("请选择一个文件夹")
        tk.messagebox.showerror("错误", "这是一个错误提示")

# 定义函数以打开文件选择对话框并设置 mydir_path_str_var
def choose_mydir():
    # 使用filedialog打开文件选择对话框，并获取选中的文件路径
    global mydir_path_str_var
    mydir_path_str_var.set(filedialog.askopenfilename().replace("/","\\"))
    if mydir_path_str_var:
        mydir_path_str_var.get()  # 或者执行其他你需要的操作
    else:
        print("请选择一个文件夹")
        tk.messagebox.showerror("错误", "这是一个错误提示")

# 定义函数以打开文件选择对话框并设置 myfile_path_str_var
def choose_myfile():
    # 使用filedialog打开文件选择对话框，并获取选中的文件路径
    global myfile_path_str_var
    myfile_path_str_var.set(filedialog.askopenfilename().replace('/', '\\'))
    if myfile_path_str_var:
        myfile_path_str_var.get()  # 或者执行其他你需要的操作
    else:
        print("请选择一个文件夹")
        tk.messagebox.showerror("错误", "这是一个错误提示")

# 根据选中的 Radiobutton 值更新界面布局
def update_layout(value):
    # 根据选中的Radiobutton值更新界面
    if value == 0:
        tk.Label(root, width=10).grid()
        tk.Label(root, text='扫描目录').grid(row=1, column=1, sticky=tk.W, pady=10)
        tk.Entry(root, textvariable=dir_path_str_var, width=50).grid(row=1, column=2, sticky=tk.W)

        button = tk.Button(root, text='执行', width=20, command=exe_0)
        button.grid(row=7, column=2, sticky=tk.W, pady=10)


    if value == 1:
        tk.Label(root, width=10).grid()

        tk.Label(root, text='扫描目录').grid(row=1, column=1, sticky=tk.W, pady=10)
        tk.Entry(root, textvariable=dir_path_str_var, width=50).grid(row=1, column=2, sticky=tk.W)

        tk.Label(root, text='对比文件').grid(row=2, column=1, sticky=tk.W, pady=10)
        tk.Entry(root, textvariable=mystat_path_str_var, width=50).grid(row=2, column=2, sticky=tk.W)
        choose_mystat_button = tk.Button(root, text='选择文件', command=choose_mystat)
        choose_mystat_button.grid(row=2, column=3, pady=10, padx=10, sticky=tk.W)

        button = tk.Button(root, text='执行', width=20, command=exe_1)
        button.grid(row=7, column=2, sticky=tk.W, pady=10)

    if value == 2:
        tk.Label(root, width=10).grid()

        tk.Label(root, text='扫描目录').grid(row=1, column=1, sticky=tk.W, pady=10)
        tk.Entry(root, textvariable=dir_path_str_var, width=50).grid(row=1, column=2, sticky=tk.W)

        tk.Label(root, text='对比文件').grid(row=2, column=1, sticky=tk.W, pady=10)
        tk.Entry(root, textvariable=mystat_path_str_var, width=50).grid(row=2, column=2, sticky=tk.W)
        choose_mystat_button = tk.Button(root, text='选择文件', command=choose_mystat)
        choose_mystat_button.grid(row=2, column=3, pady=10, padx=10, sticky=tk.W)

        tk.Label(root, text='文件夹删改文本').grid(row=3, column=1, sticky=tk.W, pady=10)
        tk.Entry(root, textvariable=mydir_path_str_var, width=50).grid(row=3, column=2, sticky=tk.W)
        choose_mydir_button = tk.Button(root, text='选择文件', command=choose_mydir)
        choose_mydir_button.grid(row=3, column=3, pady=10, padx=10, sticky=tk.W)

        button = tk.Button(root, text='执行', width=20, command=exe_2)
        button.grid(row=7, column=2, sticky=tk.W, pady=10)

    if value == 3:

        tk.Label(root, width=10).grid()

        tk.Label(root, text='扫描目录').grid(row=1, column=1, sticky=tk.W, pady=10)
        tk.Entry(root, textvariable=dir_path_str_var, width=50).grid(row=1, column=2, sticky=tk.W)

        tk.Label(root, text='对比文件').grid(row=2, column=1, sticky=tk.W, pady=10)
        tk.Entry(root, textvariable=mystat_path_str_var, width=50).grid(row=2, column=2, sticky=tk.W)
        choose_mystat_button = tk.Button(root, text='选择文件', command=choose_mystat)
        choose_mystat_button.grid(row=2, column=3, pady=10, padx=10, sticky=tk.W)

        tk.Label(root, text='文件夹删改文本').grid(row=3, column=1, sticky=tk.W, pady=10)
        tk.Entry(root, textvariable=mydir_path_str_var, width=50).grid(row=3, column=2, sticky=tk.W)
        choose_mydir_button = tk.Button(root, text='选择文件', command=choose_mydir)
        choose_mydir_button.grid(row=3, column=3, pady=10, padx=10, sticky=tk.W)

        tk.Label(root, text='文件删改文本').grid(row=4, column=1, sticky=tk.W, pady=10)
        tk.Entry(root, textvariable=myfile_path_str_var, width=50).grid(row=4, column=2, sticky=tk.W)
        choose_mydir_button = tk.Button(root, text='选择文件', command=choose_myfile)
        choose_mydir_button.grid(row=4, column=3, pady=10, padx=10, sticky=tk.W)

        button = tk.Button(root, text='执行', width=20, command=exe_3)
        button.grid(row=7, column=2, sticky=tk.W, pady=10)

# 获取当前目录和父目录
currentpath = os.getcwd()
parent_directory = os.path.dirname(currentpath)

# 加载 DLL
dllpath = r"..\C Source Code\test_ctypes\x64\Debug\test_ctypes.dll"
#dllpath=r"C:\Users\ii\source\repos\test_ctypes\x64\Debug\test_ctypes.dll"
lib = CDLL(dllpath)

# 设置窗口和界面元素
root = tk.Window(themename='solar')
dir_path_str_var = tk.StringVar()
type_int_var = tk.IntVar()
mystat_path_str_var = tk.StringVar()
mydir_path_str_var = tk.StringVar()
myfile_path_str_var = tk.StringVar()
root.geometry('1000x500+500+500')  # 设置窗口大小和位置
root.title('File Schema Analysis')  # 设置窗口标题
root.wm_attributes('-topmost', 1)  # 窗口置顶，其他的任何窗口都在其之下

# 创建 Radiobuttons 和其他界面元素
radio_frame = tk.Frame()  # 使用Frame来组织单选框
radio_frame.grid(row=5, column=2, sticky=tk.W)
# 修改Radiobutton组件，添加command参数
tk.Radiobutton(radio_frame, text='扫描文件夹', variable=type_int_var, value=0, command=on_radiobutton_select).pack(
    side=tk.LEFT, padx=5)
tk.Radiobutton(radio_frame, text='统计文件信息', variable=type_int_var, value=1, command=on_radiobutton_select).pack(
    side=tk.LEFT, padx=5)
tk.Radiobutton(radio_frame, text='删改文件夹并统计', variable=type_int_var, value=2,
               command=on_radiobutton_select).pack(side=tk.LEFT, padx=5)
tk.Radiobutton(radio_frame, text='删改文件并统计', variable=type_int_var, value=3,
               command=on_radiobutton_select).pack(side=tk.LEFT, padx=5)
root.iconbitmap(r"..\ico\favicon.ico")

tk.Label(root, text="输出").grid(row=6, column=1, sticky=tk.W, pady=10)

myout = tk.Text(root, height=10, width=50)
myout.grid(row=6, column=2, sticky=tk.W)

# 其他代码 ...
root.mainloop()
