# Parallelizing the Plane Sweep Algoirthm

My senior project at UF focused on the research of Dr. McKenney in parallelizing the plane sweep algorithm.
McKenney implemented a preprocessing step that makes parallelizing the alogirthm process possible.
The original implementation completed this preprocessing in serial across 2 regions.
I was able to create a parallelization of the preprocessing step, which greatly improves
performance on modern multi-core machines. 
