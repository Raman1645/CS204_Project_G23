import tkinter as tk
from tkinter import filedialog, scrolledtext
import subprocess

# Create GUI window
root = tk.Tk()
root.title("RISC-V Simulator")
root.geometry("700x500")  # Increased window size
root.configure(bg="#282C34")  # Dark background

file_label = tk.Label(root, text="No file selected", font=("Arial", 10), fg="white", bg="#282C34")
file_label.pack(pady=5)

def upload_file():
    global file_path
    file_path = filedialog.askopenfilename(filetypes=[("Machine Code Files", "*.mc")])
    if file_path:
        file_label.config(text=f"Selected: {file_path}", fg="#61AFEF")

def run_simulator():
    if not file_path:
        output_box.insert(tk.END, "No input.mc file selected!\n", "error")
        return
    
    # Compile newfile.cpp
    compile_process = subprocess.run(["g++", "newfile.cpp", "-o", "newfile.out"], capture_output=True, text=True)
    if compile_process.returncode != 0:
        output_box.insert(tk.END, "Compilation Error:\n" + compile_process.stderr, "error")
        return
    
    # Run the compiled C++ executable with the selected input.mc file
    execute_process = subprocess.run(["newfile.out", file_path], capture_output=True, text=True)
    output_box.insert(tk.END, execute_process.stdout if execute_process.stdout else execute_process.stderr, "output")

# Buttons with styling
upload_button = tk.Button(root, text="Upload input.mc", command=upload_file, font=("Arial", 12), fg="white", bg="#61AFEF", padx=10, pady=5, relief="ridge")
upload_button.pack(pady=10)

execute_button = tk.Button(root, text="Execute", command=run_simulator, font=("Arial", 12), fg="white", bg="#98C379", padx=10, pady=5, relief="ridge")
execute_button.pack(pady=10)

# Styled output box with increased size
output_box = scrolledtext.ScrolledText(root, wrap=tk.WORD, height=60, width=120, font=("Courier", 10), fg="#ABB2BF", bg="#1E2127", insertbackground="white")
output_box.tag_configure("error", foreground="#E06C75")
output_box.tag_configure("output", foreground="#56B6C2")
output_box.pack(padx=10, pady=10)

file_path = ""

root.mainloop()