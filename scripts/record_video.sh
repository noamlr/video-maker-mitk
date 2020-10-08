SCRIPT_EXEC=/home/guilherme/Documents/noa/cidia19/screenshot-axis-views/build/videomaker
#INPUT_3D_IMAGE_PATH=/home/guilherme/Documents/noa/cidia19/data/NII-HMV-OUT/exame-pulmao/
INPUT_3D_IMAGE_PATH=/home/guilherme/Documents/noa/cidia19/data/NII-HCPA-OUT/exame-pulmao/
#OUTPUT_2D_SLICES_PATH=/home/guilherme/Documents/noa/cidia19/data/output-2d/HMV/exame-pulmao/
OUTPUT_2D_SLICES_PATH=/home/guilherme/Documents/noa/cidia19/data/output-video/
TRANSFER_FUNCTION_PATH=/home/guilherme/Documents/noa/cidia19/data/TF/tf12_2.xml

if [ -d "$INPUT_3D_IMAGE_PATH" ] && [ -d "$OUTPUT_2D_SLICES_PATH" ];
then
	 # Deleting everything in output path
	 rm -r $OUTPUT_2D_SLICES_PATH/*
   
	 # Creating ExameI/axisJ
	 cd $INPUT_3D_IMAGE_PATH
   array_exams=($(ls -d */))
	 cd $OUTPUT_2D_SLICES_PATH
   for i in "${array_exams[@]}"
	
	 # reading all cropped lungs
	 array_lung_input=($(ls $INPUT_3D_IMAGE_PATH*/*/crop_by_mask_*))

	 for i in "${array_lung_input[@]}"
	 do
	 		# echo $
			array_split=(${i//\// })
			echo ${array_split[8]} 
			OUTPUT_PATH="$OUTPUT_2D_SLICES_PATH$/{array_split[8]}.mp4"
			echo $OUTPUT_PATH
			$SCRIPT_EXEC -tf $TRANSFER_FUNCTION_PATH -i $i -o $OUTPUT_PATH
	 done

else
    echo "Error: $INPUT_3D_IMAGE_PATH or $OUTPUT_2D_SLICES_PATH doesn't exists"
fi