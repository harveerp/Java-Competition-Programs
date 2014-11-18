/**
 * Created with IntelliJ IDEA.
 * User: curtis
 * Date: 2013-03-14
 * Time: 7:17 PM
 * To change this template use File | Settings | File Templates.
 */

/*
 * Main.java
 *  java program model for www.programming-challenges.com
*/

import java.util.ArrayList;
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
    private int maxDepth, arrayIteration;
    private ArrayList<Integer> list;

    public void run() {


        Scanner lineReader;
        int tests = 0;
        int number;

        //lineReader = new Scanner(scan.nextLine());

        number = scan.nextInt();

        // Loop for whole program
        while (number != -1)
        {

            tests ++;

            list  = new ArrayList<Integer>();

            // Start output for current test
            output.append("Test #" + tests + "\n   maximum possible interceptions: ");


            // Loop for inputting numbers for each test case
            while (number != -1)
            {

                list.add(number);

                number = scan.nextInt();

            }

            findInterceptions();

            output.append("\n\n");

            number = scan.nextInt();

        }


        System.out.print(output.toString());
    }

    private void findInterceptions()
    {

        // Set up variables
        //int currentDepth = 0;
        maxDepth = 0;
        arrayIteration = 1;

        while (arrayIteration < list.size())
        {
            nextDepth(1);
            arrayIteration ++;
        }


        output.append(maxDepth);


    }

    private void nextDepth(int currentDepth)
    {

        int currentListNumber = list.get(arrayIteration - 1);

        if(currentDepth > maxDepth) maxDepth = currentDepth;

        while (arrayIteration < list.size())
        {

            if(currentListNumber >= list.get(arrayIteration))
            {

                arrayIteration ++;
                nextDepth(currentDepth + 1);

            }
            else break;

        }

    }

}

