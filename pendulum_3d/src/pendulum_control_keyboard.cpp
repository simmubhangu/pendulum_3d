    #include <termios.h>  

    #include <signal.h>  

    #include <math.h>  

    #include <stdio.h>  

    #include <stdlib.h>  

    #include <sys/poll.h>  

     

    #include <boost/thread/thread.hpp>  

    #include <ros/ros.h>  

    #include <geometry_msgs/Twist.h>  

    #include "std_msgs/Float64.h"

     

    #define KEYCODE_W 0x77  

    #define KEYCODE_A 0x61  

    #define KEYCODE_S 0x73  

    #define KEYCODE_D 0x64  

    #define KEYCODE_H 0x68 

     

    #define KEYCODE_A_CAP 0x41  

    #define KEYCODE_D_CAP 0x44  

    #define KEYCODE_S_CAP 0x53  

    #define KEYCODE_W_CAP 0x57  

     

    class SmartCarKeyboardTeleopNode  

    {  

        private:  

            double walk_vel_;  

            double run_vel_;  

            double yaw_rate_;  

            double yaw_rate_run_;  

     

            geometry_msgs::Twist cmdvel_;  

            ros::NodeHandle n_;  

            ros::Publisher pub_; 
		
	    ros::Publisher pub2_; 

     

        public:  

            SmartCarKeyboardTeleopNode()  

            {  

                pub_ = n_.advertise<std_msgs::Float64>("/pendulum_3d/control_x_controller/command", 5);

		pub2_ = n_.advertise<std_msgs::Float64>("/pendulum_3d/control_y_controller/command", 5);

                ros::NodeHandle n_private("~");  

                n_private.param("walk_vel", walk_vel_, 0.5);  

                n_private.param("run_vel", run_vel_, 1.0);  

                n_private.param("yaw_rate", yaw_rate_, 1.0);  

                n_private.param("yaw_rate_run", yaw_rate_run_, 1.5);  

            }  

     

            ~SmartCarKeyboardTeleopNode() { }  

            void keyboardLoop();  

     

            void stopRobot()  

            {  

                cmdvel_.linear.x = 0.0;  

                cmdvel_.angular.z = 0.0;  

                pub_.publish(cmdvel_);  

            }  

    };  

     

    SmartCarKeyboardTeleopNode* tbk;  

    int kfd = 0;  

    struct termios cooked, raw;  

    bool done;  

     

    int main(int argc, char** argv)  

    {  

        ros::init(argc,argv,"tbk", ros::init_options::AnonymousName | ros::init_options::NoSigintHandler);  

        SmartCarKeyboardTeleopNode tbk;  

     

        boost::thread t = boost::thread(boost::bind(&SmartCarKeyboardTeleopNode::keyboardLoop, &tbk));  

     

        ros::spin();  

     

        t.interrupt();  

        t.join();  

        tbk.stopRobot();  

        tcsetattr(kfd, TCSANOW, &cooked);  

     

        return(0);  

    }  

     

    void SmartCarKeyboardTeleopNode::keyboardLoop()  

    {  

        char c;  

        double max_tv = walk_vel_;  

        double max_rv = yaw_rate_;  

        bool dirty = false;  

        int speed = 0;  

        int turn = 0;  

	std_msgs::Float64 cmdvel;
	
	cmdvel.data = 0;

	std_msgs::Float64 cmdvel2;
	
	cmdvel2.data = 0;

     

        // get the console in raw mode  

        tcgetattr(kfd, &cooked);  

        memcpy(&raw, &cooked, sizeof(struct termios));  

        raw.c_lflag &=~ (ICANON | ECHO);  

        raw.c_cc[VEOL] = 1;  

        raw.c_cc[VEOF] = 2;  

        tcsetattr(kfd, TCSANOW, &raw);  

     

        puts("Reading from keyboard");  

        puts("Use WASD keys to control the robot");  

        puts("Press Shift to move faster");  

     

        struct pollfd ufd;  

        ufd.fd = kfd;  

        ufd.events = POLLIN;  

     

        for(;;)  

        {  

            boost::this_thread::interruption_point();  

     

            // get the next event from the keyboard  

            int num;  

     

            if ((num = poll(&ufd, 1, 250)) < 0)  

            {  

                perror("poll():");  

                return;  

            }  

            else if(num > 0)  

            {  

                if(read(kfd, &c, 1) < 0)  

                {  

                    perror("read():");  

                    return;  

                }  

            }  

            else  

            {  

                if (dirty == true)  

                {  

                    stopRobot();  

                    dirty = false;  

                }  

     

                continue;  

            }  

     

            switch(c)  

            {  

                case KEYCODE_W:  

		    cmdvel.data = cmdvel.data + 0.1;

                    break;  

                case KEYCODE_S:
  
		    cmdvel.data = cmdvel.data - 0.1; 

                    break;  

                case KEYCODE_A:  

		    cmdvel2.data = cmdvel2.data + 0.1;

                    break;  

                case KEYCODE_D:
  
		    cmdvel2.data = cmdvel2.data - 0.1; 

                    break;     

                case KEYCODE_H:

		    cmdvel.data = 0;
  
		    cmdvel2.data = 0; 

                    break;          

                default:  

		    cmdvel.data = cmdvel.data;

		    cmdvel2.data = cmdvel2.data;

            }   

            pub_.publish(cmdvel);  

	    pub2_.publish(cmdvel2);  

        }  

    }
