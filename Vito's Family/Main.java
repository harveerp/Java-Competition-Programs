import java.util.ArrayList;
import java.util.Collections;
import java.util.Scanner;

/**
 * Created with IntelliJ IDEA.
 * User: harveerparmar
 * Date: 2013-03-11
 * Time: 6:01 PM
 * To change this template use File | Settings | File Templates.
 */


public class Main implements Runnable{


    public static void main(String args[])  // entry point from OS
    {
        Main myWork = new Main();  // Construct the bootloader
        myWork.run();            // execute
    }

    public void run() {
        new myStuff().run();
    }
}
class myStuff implements Runnable{

    private StringBuilder output = new StringBuilder();
    private Scanner scanner = new Scanner(System.in);
    ArrayList<Integer> houseNumbers = new ArrayList<Integer>();

    public void run(){

        Scanner lineReader;
        int testCases;
        lineReader = new Scanner(scanner.nextLine());
        testCases = lineReader.nextInt();

    for(int j = 0; j < testCases; j++){
        int minDistance = 0;
        int middle = 0;
        int median = 0;
        int counter = 0;
        int housenumber;
        int firstNumber;
        int secondNumber;


        lineReader = new Scanner(scanner.nextLine());
        int house =lineReader.nextInt();

        while(house != counter) {
            housenumber = lineReader.nextInt();
            houseNumbers.add(housenumber);
            counter++;
        }

        Collections.sort(houseNumbers);

        if(houseNumbers.size() % 2 == 0) {
            firstNumber = houseNumbers.size() / 2 - 1;
            secondNumber = houseNumbers.size() / 2;
            middle = (houseNumbers.get(firstNumber) + houseNumbers.get(secondNumber)) / 2;
        }
        else{
            median = houseNumbers.size() / 2;
            middle = houseNumbers.get(median);
        }

        for(int i = 0;i < houseNumbers.size();i++){
            minDistance += Math.abs(middle - houseNumbers.get(i));
        }
        output.append(minDistance);
        houseNumbers = new ArrayList<Integer>();

        if(j != testCases - 1)
            output.append("\n");

        }
        System.out.println(output.toString());
    }
}