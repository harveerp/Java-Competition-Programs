import java.util.Scanner;

/**
 * Created with IntelliJ IDEA.
 * User: harveerparmar
 * Date: 2013-03-13
 * Time: 7:12 PM
 * To change this template use File | Settings | File Templates.
 */
public class Main implements Runnable {

    public static void main(String[] args){
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

        //String first = "`1234567890-QWERTYUIOP[]ASDFGHJKL;ZXCVBNM,.";
       // String second = "1234567890-=WERTYUIOP[]\\SDFGHJKL;'XCVBNM,./";
        String all = "`1234567890-=QWERTYUIOP[]\\ASDFGHJKL;'ZXCVBNM,./";

        while(scanner.hasNextLine()){
            Scanner lineReader;
            lineReader = new Scanner(scanner.nextLine());

            String input = lineReader.nextLine();
            StringBuilder output = new StringBuilder();


            for(int i = 0;i < input.length();i++){
                if(input.charAt(i) == ' '){
                    output.append(" ");
                }
                else{
                    if((all.indexOf(input.charAt(i)) - 1 == -1))
                        output.append(all.charAt(all.length() - 1));
                    else
                        output.append(all.charAt(all.indexOf(input.charAt(i)) - 1));
                }

            }
        System.out.println(output.toString());
        }
        }
    }
