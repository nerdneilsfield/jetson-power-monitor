    /* Print power statistics */
    printf("\n功耗统计信息:\n");
    printf("总功耗:\n");
    printf("  最小值: %.2f W\n", stats.total.power.min);
    printf("  最大值: %.2f W\n", stats.total.power.max);
    printf("  平均值: %.2f W\n", stats.total.power.avg);
    printf("  总能耗: %.2f J\n", stats.total.power.avg * total_time);
    printf("  采样次数: %ld\n", stats.total.power.count);

    printf("\n各传感器功耗信息:\n");
    for (int i = 0; i < stats.sensor_count; i++) {
        printf("\n传感器: %s\n", stats.sensors[i].name);
        printf("  最小值: %.2f W\n", stats.sensors[i].power.min);
        printf("  最大值: %.2f W\n", stats.sensors[i].power.max);
        printf("  平均值: %.2f W\n", stats.sensors[i].power.avg);
        printf("  总能耗: %.2f J\n", stats.sensors[i].power.avg * total_time);
        printf("  采样次数: %ld\n", stats.sensors[i].power.count);
    } 