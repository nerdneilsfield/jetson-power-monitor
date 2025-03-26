#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import jetpwmon
import time

class TestJetPwMon(unittest.TestCase):
    def setUp(self):
        """Setup before each test case"""
        self.monitor = jetpwmon.PowerMonitor()
        
    def tearDown(self):
        """Cleanup after each test case"""
        if hasattr(self, 'monitor'):
            del self.monitor

    def test_init(self):
        """Test PowerMonitor initialization"""
        self.assertIsNotNone(self.monitor)
        
    def test_sampling_frequency(self):
        """Test setting and getting sampling frequency"""
        # Test setting sampling frequency
        test_freq = 10
        self.monitor.set_sampling_frequency(test_freq)
        
        # Test getting sampling frequency
        actual_freq = self.monitor.get_sampling_frequency()
        self.assertEqual(actual_freq, test_freq)
        
        # Test invalid sampling frequency
        with self.assertRaises(RuntimeError):
            self.monitor.set_sampling_frequency(0)
            
    def test_sampling_control(self):
        """Test sampling control functions"""
        # Test starting sampling
        self.monitor.start_sampling()
        self.assertTrue(self.monitor.is_sampling())
        
        # Test duplicate start sampling
        with self.assertRaises(RuntimeError):
            self.monitor.start_sampling()
            
        # Test stopping sampling
        self.monitor.stop_sampling()
        self.assertFalse(self.monitor.is_sampling())
        
        # Test duplicate stop sampling
        with self.assertRaises(RuntimeError):
            self.monitor.stop_sampling()
            
    def test_data_collection(self):
        """Test power data collection"""
        # Set sampling frequency and start sampling
        self.monitor.set_sampling_frequency(10)
        self.monitor.start_sampling()
        
        # Wait for some data to be collected
        time.sleep(0.5)
        
        # Get latest data
        data = self.monitor.get_latest_data()
        self.assertIsInstance(data, dict)
        self.assertIn('total', data)
        self.assertIn('sensors', data)
        self.assertIn('sensor_count', data)
        
        # Check total data
        total = data['total']
        self.assertIn('voltage', total)
        self.assertIn('current', total)
        self.assertIn('power', total)
        self.assertIn('online', total)
        self.assertIn('status', total)
        
        # Check sensor data
        sensors = data['sensors']
        self.assertIsInstance(sensors, list)
        if sensors:  # If sensors exist
            sensor = sensors[0]
            self.assertIn('name', sensor)
            self.assertIn('type', sensor)
            self.assertIn('voltage', sensor)
            self.assertIn('current', sensor)
            self.assertIn('power', sensor)
            self.assertIn('online', sensor)
            self.assertIn('status', sensor)
            self.assertIn('warning_threshold', sensor)
            self.assertIn('critical_threshold', sensor)
            
        # Stop sampling
        self.monitor.stop_sampling()
        
    def test_statistics(self):
        """Test power statistics collection"""
        # Reset statistics
        self.monitor.reset_statistics()
        
        # Set sampling frequency and start sampling
        self.monitor.set_sampling_frequency(10)
        self.monitor.start_sampling()
        
        # Wait for some data to be collected
        time.sleep(0.5)
        
        # Get statistics
        stats = self.monitor.get_statistics()
        self.assertIsInstance(stats, dict)
        self.assertIn('total', stats)
        self.assertIn('sensors', stats)
        self.assertIn('sensor_count', stats)
        
        # Check total statistics
        total = stats['total']
        for key in ['voltage', 'current', 'power']:
            self.assertIn(key, total)
            stat_data = total[key]
            self.assertIn('min', stat_data)
            self.assertIn('max', stat_data)
            self.assertIn('avg', stat_data)
            self.assertIn('total', stat_data)
            self.assertIn('count', stat_data)
            
        # Check sensor statistics
        sensors = stats['sensors']
        self.assertIsInstance(sensors, list)
        if sensors:  # If sensors exist
            sensor = sensors[0]
            self.assertIn('name', sensor)
            for key in ['voltage', 'current', 'power']:
                self.assertIn(key, sensor)
                stat_data = sensor[key]
                self.assertIn('min', stat_data)
                self.assertIn('max', stat_data)
                self.assertIn('avg', stat_data)
                self.assertIn('total', stat_data)
                self.assertIn('count', stat_data)
                
        # Stop sampling
        self.monitor.stop_sampling()
        
    def test_sensor_info(self):
        """Test sensor information retrieval"""
        # Get sensor count
        count = self.monitor.get_sensor_count()
        self.assertIsInstance(count, int)
        self.assertGreaterEqual(count, 0)
        
        # Get sensor names
        names = self.monitor.get_sensor_names()
        self.assertIsInstance(names, list)
        self.assertEqual(len(names), count)
        for name in names:
            self.assertIsInstance(name, str)
            
    def test_error_handling(self):
        """Test error handling functionality"""
        # Test error codes
        self.assertEqual(jetpwmon.ErrorCode.SUCCESS, 0)
        self.assertEqual(jetpwmon.ErrorCode.ERROR_INIT_FAILED, -1)
        
        # Test error strings
        error_msg = jetpwmon.error_string(jetpwmon.ErrorCode.SUCCESS)
        self.assertIsInstance(error_msg, str)
        self.assertGreater(len(error_msg), 0)
        
    def test_sensor_types(self):
        """Test sensor type enumeration"""
        # Check sensor type values
        self.assertEqual(jetpwmon.SensorType.UNKNOWN, 0)
        self.assertEqual(jetpwmon.SensorType.I2C, 1)
        self.assertEqual(jetpwmon.SensorType.SYSTEM, 2)

if __name__ == '__main__':
    unittest.main()
