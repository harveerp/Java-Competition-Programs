import java.util.Scanner;

/**
 * Created with IntelliJ IDEA.
 * User: harveerparmar
 * Date: 2013-02-23
 * Time: 5:40 PM
 * To change this template use File | Settings | File Templates.
 */
public class Main implements Runnable {

    public static void main(String args[]){

           Main myWork = new Main();
           myWork.run();


    }

    public void run(){
        new myStuff().run();

    }
}


class myStuff implements Runnable{
    private Scanner scanner = new Scanner(System.in);
    private StringBuilder output = new StringBuilder();
    int testCases;
    double xSteps;
    double ySteps;

    public void run(){

        Scanner lineReader;
        lineReader = new Scanner(scanner.nextLine());
        int steps;

        testCases = lineReader.nextInt();

        for(int i=0;i < testCases;i++){

            lineReader = new Scanner(scanner.nextLine());

            while(lineReader.hasNextInt()){
                xSteps = lineReader.nextInt();
                ySteps = lineReader.nextInt();
            }

            steps = (int) (ySteps - xSteps);

            if(steps <= 3)
                output.append(steps);
            else{
                int x;
                x = (int) Math.sqrt(steps);

                if(Math.pow(x,2) == steps)
                    output.append(2 * x - 1);
                else if((steps - Math.pow(x,2) <= x))
                    output.append(2 * x);
                else
                    output.append(2 * x + 1);
            }
            if(i != testCases - 1)
                output.append("\n");
        }
        System.out.println(output.toString());


    }
}

