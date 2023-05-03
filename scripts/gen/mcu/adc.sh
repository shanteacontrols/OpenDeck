#!/usr/bin/env bash

declare -i adc_prescaler
declare -i adc_samples

adc_prescaler=$($yaml_parser "$project_yaml_file" adc.prescaler)
adc_samples=$($yaml_parser "$project_yaml_file" adc.samples)

if [[ $adc_prescaler != "null" ]]
then
    printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_ADC_PRESCALER=$adc_prescaler" >> "$out_makefile"
fi

if [[ $adc_samples != "null" ]]
then
    printf "%s\n" "PROJECT_MCU_DEFINES += PROJECT_MCU_ADC_SAMPLES=$adc_samples" >> "$out_makefile"
fi