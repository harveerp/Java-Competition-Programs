import java.math.BigInteger;
import java.util.Scanner;

/**
 * Created with IntelliJ IDEA.
 * User: harveerparmar
 * Date: 2013-03-18
 * Time: 6:07 PM
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


    private Scanner scanner = new Scanner(System.in);
    public void run(){

        int testCases;

        String first;
        String second;
        Scanner lineReader;
        lineReader = new Scanner(scanner.nextLine());



        testCases = lineReader.nextInt();

        for(int i = 0;i < testCases;i++){
            StringBuilder output = new StringBuilder();

            first = scanner.next();
            second = scanner.next();

            BigInteger[][] subsequences = new BigInteger[first.length() + 1][second.length() + 1];

            int counter = 0;
            while(counter <= first.length()){
                subsequences[counter][0] = BigInteger.ONE;
                counter++;
            }

            int counter2 = 1;
            while(counter2 <= second.length()){
                subsequences[0][counter2] = BigInteger.ZERO;
                counter2++;
            }

            for(int j = 1;j <= first.length();j++){
                for(int k= 1;k <= second.length();k++){

                    if(first.charAt(j - 1) == second.charAt(k - 1)){
                        subsequences[j][k] = subsequences[j - 1][k].add(subsequences[j - 1][k - 1]);
                    }
                    else if(first.charAt(j - 1) != second.charAt(k - 1))
                        subsequences[j][k] = subsequences[j - 1][k];
                }
            }

            output.append(subsequences[first.length()][second.length()]);
            System.out.println(output.toString());

        }
    }
}
