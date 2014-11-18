import java.util.Scanner;

/**
 * Created with IntelliJ IDEA.
 * User: harveerparmar
 * Date: 2013-04-07
 * Time: 7:24 PM
 * To change this template use File | Settings | File Templates.
 */
public class Main implements Runnable {

    public static void main(String[] arg){
        Main myWork = new Main();
        myWork.run();
    }

    public void run(){
        new myStuff().run();
    }
}

class myStuff implements Runnable{


    public void run(){

        Scanner scanner = new Scanner(System.in);

        double sideLength;
        double striped;
        double dotted;
        double rest;
        double pie = 2.0 * Math.acos(0.0);
        double root = Math.sqrt(3);

        StringBuilder output = new StringBuilder();

        while(scanner.hasNextDouble()){
            sideLength = scanner.nextDouble();
            double side = sideLength * sideLength;
            striped = (pie / 3 + 1 - root) * side;
            dotted = (pie / 3 - 4 + 2 * root) * side;
            rest = (4 - (2 * pie) / 3 - root) * side;

            output.append(String.format("%.3f %.3f %.3f%n",striped,dotted,rest));
        }
       System.out.println(output.toString());
    }
}
