import streamlit as st
import time
import pandas as pd
import matplotlib.pyplot as plt
import plotly.graph_objects as go
from processor import Processor
from utils import parse_instruction_file, parse_data_file

st.set_page_config(
    page_title="Pipelined Processor Simulator",
    layout="wide"
)

# Initialize session state for processor if not exists
if 'processor' not in st.session_state:
    st.session_state.processor = None

if 'cycle_number' not in st.session_state:
    st.session_state.cycle_number = 0
    
if 'simulation_finished' not in st.session_state:
    st.session_state.simulation_finished = False
    
if 'instruction_trace' not in st.session_state:
    st.session_state.instruction_trace = []
    
if 'hazards' not in st.session_state:
    st.session_state.hazards = []
    
if 'statistics' not in st.session_state:
    st.session_state.statistics = {}

def reset_simulation():
    st.session_state.processor = None
    st.session_state.cycle_number = 0
    st.session_state.simulation_finished = False
    st.session_state.instruction_trace = []
    st.session_state.hazards = []
    st.session_state.statistics = {}

def initialize_processor(instruction_file, data_file, knobs):
    # Parse instructions and data
    instructions = parse_instruction_file(instruction_file)
    data = parse_data_file(data_file)
    
    # Create processor
    processor = Processor(
        instructions=instructions,
        data=data,
        enable_pipeline=knobs["knob1"],
        enable_forwarding=knobs["knob2"],
        enable_reg_trace=knobs["knob3"],
        enable_pipeline_trace=knobs["knob4"],
        trace_instruction=knobs["knob5_instruction"],
        enable_branch_trace=knobs["knob6"]
    )
    
    return processor

def run_one_cycle():
    if st.session_state.processor and not st.session_state.simulation_finished:
        # Execute one cycle
        finished = st.session_state.processor.execute_cycle()
        
        # Update cycle counter
        st.session_state.cycle_number += 1
        
        # Update instruction trace
        st.session_state.instruction_trace = st.session_state.processor.get_instruction_trace()
        
        # Update hazards
        st.session_state.hazards = st.session_state.processor.get_hazards()
        
        if finished:
            st.session_state.simulation_finished = True
            st.session_state.statistics = st.session_state.processor.get_statistics()

def run_full_simulation():
    if st.session_state.processor and not st.session_state.simulation_finished:
        while not st.session_state.simulation_finished:
            run_one_cycle()

def visualize_pipeline():
    if not st.session_state.processor:
        return
    
    # Get current pipeline state
    pipeline_state = st.session_state.processor.get_pipeline_state()
    
    # Create a figure for pipeline visualization
    fig = go.Figure()
    
    # Define the pipeline stages
    stages = ["Fetch", "Decode", "Execute", "Memory", "Writeback"]
    
    # Add boxes for each stage
    for i, stage in enumerate(stages):
        instruction = pipeline_state.get(stage, "")
        fig.add_shape(
            type="rect",
            x0=i, 
            y0=0, 
            x1=i+0.9, 
            y1=1,
            line=dict(color="RoyalBlue", width=2),
            fillcolor="LightSkyBlue" if instruction else "White",
        )
        fig.add_annotation(
            x=i+0.45,
            y=0.5,
            text=f"{stage}<br>{instruction}",
            showarrow=False,
            font=dict(size=12)
        )
    
    # Add pipeline registers
    for i in range(4):
        fig.add_shape(
            type="rect",
            x0=i+0.9, 
            y0=0, 
            x1=i+1, 
            y1=1,
            line=dict(color="Red", width=2),
            fillcolor="LightPink",
        )
    
    # Add forwarding paths if enabled and active
    if st.session_state.processor.enable_forwarding:
        forwarding_paths = st.session_state.processor.get_forwarding_paths()
        for path in forwarding_paths:
            fig.add_shape(
                type="line",
                x0=path["from_x"], 
                y0=path["from_y"], 
                x1=path["to_x"], 
                y1=path["to_y"],
                line=dict(color="Green", width=2, dash="dash"),
            )
            fig.add_annotation(
                x=(path["from_x"] + path["to_x"]) / 2,
                y=(path["from_y"] + path["to_y"]) / 2,
                text=path["description"],
                showarrow=False,
                font=dict(size=10, color="Green")
            )
    
    # Set layout
    fig.update_layout(
        title=f"Pipeline State - Cycle {st.session_state.cycle_number}",
        xaxis=dict(showticklabels=False, showgrid=False, zeroline=False, range=[-0.1, 5.1]),
        yaxis=dict(showticklabels=False, showgrid=False, zeroline=False, range=[-0.1, 1.1]),
        width=1000,
        height=300,
        margin=dict(l=20, r=20, t=50, b=20),
    )
    
    return fig

def visualize_branch_predictor():
    if not st.session_state.processor or not st.session_state.processor.enable_branch_trace:
        return None
    
    bp_data = st.session_state.processor.get_branch_predictor_state()
    if not bp_data:
        return None
    
    # Create dataframe for branch predictor table
    bpt_df = pd.DataFrame(bp_data.get("branch_target_buffer", []))
    pht_df = pd.DataFrame(bp_data.get("pattern_history_table", []))
    
    return bpt_df, pht_df

def display_statistics():
    if not st.session_state.statistics:
        return
    
    stats = st.session_state.statistics
    
    # Display statistics in two columns
    col1, col2 = st.columns(2)
    
    with col1:
        st.subheader("General Statistics")
        st.metric("Total Cycles", stats.get("total_cycles", 0))
        st.metric("Instructions Executed", stats.get("total_instructions", 0))
        st.metric("CPI", f"{stats.get('cpi', 0):.2f}")
        
        st.subheader("Instruction Breakdown")
        instruction_data = {
            "Type": ["Data Transfer", "ALU", "Control"],
            "Count": [
                stats.get("data_transfer_instructions", 0),
                stats.get("alu_instructions", 0),
                stats.get("control_instructions", 0)
            ]
        }
        instruction_df = pd.DataFrame(instruction_data)
        
        # Create a bar chart for instruction types
        fig, ax = plt.subplots(figsize=(8, 4))
        ax.bar(instruction_df["Type"], instruction_df["Count"])
        ax.set_title("Instruction Type Distribution")
        ax.set_ylabel("Count")
        st.pyplot(fig)
    
    with col2:
        st.subheader("Hazards and Stalls")
        st.metric("Total Stalls", stats.get("total_stalls", 0))
        st.metric("Data Hazards", stats.get("data_hazards", 0))
        st.metric("Control Hazards", stats.get("control_hazards", 0))
        st.metric("Branch Mispredictions", stats.get("branch_mispredictions", 0))
        st.metric("Stalls due to Data Hazards", stats.get("stalls_data_hazards", 0))
        st.metric("Stalls due to Control Hazards", stats.get("stalls_control_hazards", 0))
        
        # Create a pie chart for stalls
        stall_data = {
            "Type": ["Data Hazards", "Control Hazards", "Other"],
            "Count": [
                stats.get("stalls_data_hazards", 0),
                stats.get("stalls_control_hazards", 0),
                stats.get("total_stalls", 0) - 
                stats.get("stalls_data_hazards", 0) - 
                stats.get("stalls_control_hazards", 0)
            ]
        }
        
        # Only create the pie chart if there are stalls
        if stats.get("total_stalls", 0) > 0:
            fig, ax = plt.subplots(figsize=(8, 4))
            ax.pie(stall_data["Count"], labels=stall_data["Type"], autopct='%1.1f%%')
            ax.set_title("Stall Distribution")
            st.pyplot(fig)

# Main app layout
st.title("Pipelined Processor Simulator")

# Sidebar for configuration
with st.sidebar:
    st.header("Configuration")
    
    # File uploaders
    instruction_file = st.file_uploader("Upload instruction file", type=["txt"])
    data_file = st.file_uploader("Upload data file", type=["txt"])
    
    # Knobs
    st.subheader("Simulation Knobs")
    
    knob1 = st.checkbox("Knob1: Enable pipelining", value=True)
    knob2 = st.checkbox("Knob2: Enable data forwarding", value=True)
    knob3 = st.checkbox("Knob3: Show register file contents", value=False)
    knob4 = st.checkbox("Knob4: Show pipeline register contents", value=True)
    
    enable_knob5 = st.checkbox("Knob5: Trace specific instruction", value=False)
    knob5_instruction = None
    if enable_knob5:
        knob5_instruction = st.number_input("Instruction number to trace", min_value=1, value=1)
    
    knob6 = st.checkbox("Knob6: Show branch prediction details", value=True)
    
    # Collect knobs into a dictionary
    knobs = {
        "knob1": knob1,
        "knob2": knob2,
        "knob3": knob3,
        "knob4": knob4,
        "knob5_instruction": knob5_instruction if enable_knob5 else None,
        "knob6": knob6
    }
    
    # Initialize and run buttons
    if st.button("Initialize Processor"):
        if instruction_file and data_file:
            st.session_state.processor = initialize_processor(instruction_file, data_file, knobs)
            st.success("Processor initialized!")
        else:
            st.error("Please upload both instruction and data files.")
    
    col1, col2 = st.columns(2)
    with col1:
        if st.button("Run One Cycle"):
            run_one_cycle()
    
    with col2:
        if st.button("Run Full Simulation"):
            run_full_simulation()
    
    if st.button("Reset Simulation"):
        reset_simulation()

# Main content
if st.session_state.processor:
    # Pipeline visualization
    st.header("Pipeline Visualization")
    pipeline_fig = visualize_pipeline()
    if pipeline_fig:
        st.plotly_chart(pipeline_fig, use_container_width=True)
    
    # Current cycle info
    st.subheader(f"Current Cycle: {st.session_state.cycle_number}")
    
    # Register contents
    if knob3 and st.session_state.processor.get_register_file():
        st.subheader("Register File Contents")
        reg_df = pd.DataFrame(st.session_state.processor.get_register_file())
        st.dataframe(reg_df)
    
    # Pipeline register contents
    if knob4 and st.session_state.processor.get_pipeline_registers():
        st.subheader("Pipeline Register Contents")
        pipe_reg_df = pd.DataFrame(st.session_state.processor.get_pipeline_registers())
        st.dataframe(pipe_reg_df)
    
    # Branch prediction information
    if knob6:
        bp_data = visualize_branch_predictor()
        if bp_data:
            st.subheader("Branch Prediction Unit")
            st.write("Branch Target Buffer (BTB)")
            st.dataframe(bp_data[0])
            st.write("Pattern History Table (PHT)")
            st.dataframe(bp_data[1])
    
    # Display instruction trace
    st.header("Instruction Trace")
    if st.session_state.instruction_trace:
        trace_df = pd.DataFrame(st.session_state.instruction_trace)
        st.dataframe(trace_df)
    
    # Display hazards
    if st.session_state.hazards:
        st.header("Hazards")
        hazard_df = pd.DataFrame(st.session_state.hazards)
        st.dataframe(hazard_df)
    
    # Display statistics if simulation is finished
    if st.session_state.simulation_finished:
        st.header("Simulation Statistics")
        display_statistics()
else:
    st.info("Please initialize the processor to start the simulation.")
