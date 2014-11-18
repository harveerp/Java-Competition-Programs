/**
 * Created with IntelliJ IDEA.
 * User: curtis
 * Date: 2013-04-02
 * Time: 1:07 PM
 * To change this template use File | Settings | File Templates.
 */

/*
 * Main.java
 *  java program model for www.programming-challenges.com
*/

import java.math.RoundingMode;
import java.text.DecimalFormat;
import java.util.Scanner;

class Main implements Runnable {

    public static void main(String args[])  // entry point from OS
    {
        Main myWork = new Main();  // Construct the bootloader
        myWork.run();            // execute
    }

    public void run() {
        new myStuff().run();
    }
}

class myStuff implements Runnable {

    private StringBuilder output = new StringBuilder();
    private Scanner scan = new Scanner(System.in);
    private double[][] coordinates;
    private int points;


    public void run() {


        Scanner lineReader;
        boolean firstRun = true;

        // Takes in first value to be read
        lineReader = new Scanner(scan.nextLine());
        points = lineReader.nextInt();

        // Loops for each case until we get 0 for the number of points
        while (points != 0)
        {
            // Adds a return to the output string every time it cycles thought after the first run
            if (firstRun)
            {
                output.append("\n");
                firstRun = false;
            }

            // Makes new data structure to store coordinates
            coordinates = new double[points][2];

            // Reads in points and stores them into the data structure
            for (int i = 0; i < points; i++)
            {

                // Reads in next line
                lineReader = new Scanner(scan.nextLine());

                // Saves coordinates
                coordinates[i][0] = lineReader.nextDouble();
                coordinates[i][1] = lineReader.nextDouble();

            }


            // Finds closest points
            findClosestPoint();

            // Looks for next input to state whether or not to continue
            lineReader = new Scanner(scan.nextLine());
            points = lineReader.nextInt();

            if (points != 0)
                output.append('\n');

        }



        System.out.print(output.toString());
    }


    private void findClosestPoint()
    {

        double minDistance = 1000000000;
        double currentDistance;
        DecimalFormat format = new DecimalFormat("0.0000");
        format.setRoundingMode(RoundingMode.HALF_UP);

        // Loop for going through the coordinates
        for(int i = 0; i < points; i++)
        {

            for(int j = (i+1); j < points; j++)
            {

                 currentDistance = findDistanceWithOutSquared(coordinates[i][0], coordinates[i][1], coordinates[j][0], coordinates[j][1]);

                if(minDistance > currentDistance) minDistance = currentDistance;

            }

        }

        minDistance = Math.sqrt(minDistance);



        if( minDistance < 10000)
            output.append(format.format(minDistance));

        else
            output.append("INFINITY");




    }

    private double findDistanceWithOutSquared(double x1, double y1, double x2, double y2)
    {


        return(Math.pow((x1 - x2), 2) + Math.pow((y1 - y2), 2));


    }

}

